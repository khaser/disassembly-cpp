#include "command_types.h"

#include "typedefs.h"
#include <string>
#include <vector>
#include <stdexcept>

void extend_sign(int& x, int sz) {
    for (; sz < 32; ++sz) {
        if (x & (1ll << sz)) {
            x |= (1ll << (sz + 1));
        }
    }
}

std::string prettify_reg(unsigned char reg) {
    if (reg >= 32) throw std::logic_error("Accessing to non-existent register");
    std::vector<std::string> names = {
        "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1",
    };
    for (int i = 0; i <= 7; ++i) {
        names.push_back("a" + std::to_string(i));
    }
    for (int i = 2; i <= 11; ++i) {
        names.push_back("s" + std::to_string(i));
    }
    for (int i = 3; i <= 6; ++i) {
        names.push_back("t" + std::to_string(i));
    }
    return names[reg];
}

InstructionType::InstructionType(const Elf32_Word& cmd) {
    opcode = cmd & 0x7f; 
}

unsigned char InstructionType::get_rd(Elf32_Word x) {
    return get_reg(x, 7);
}

unsigned char InstructionType::get_rs1(Elf32_Word x) {
    return get_reg(x, 15);
}

unsigned char InstructionType::get_rs2(Elf32_Word x) {
    return get_reg(x, 20);
}

unsigned char InstructionType::get_funct3(Elf32_Word x) {
    return get_blk(x, 3, 12);
}

unsigned char InstructionType::get_funct7(Elf32_Word x) {
    return get_blk(x, 7, 25);
}

unsigned char InstructionType::get_reg(Elf32_Word x, unsigned char pos) {
    return get_blk(x, 5, pos);
}

unsigned char InstructionType::get_cmd(Elf32_Word x) {
    return get_blk(x, 7, 0);
}

Elf32_Word InstructionType::get_blk(Elf32_Word x, unsigned char len, unsigned char pos) {
    return (x>>pos)&((1ll << len) - 1);
}

UType::UType(const Elf32_Word& cmd) : InstructionType(cmd) {
    rd = get_rd(cmd);
    imm = get_blk(cmd, 20, 12) << 12;
    cmd_name = (get_cmd(cmd) == LUI ? "lui" : "auipc");
}

void UType::print() {
    printf("%7s\t%s, %x", cmd_name.c_str(), prettify_reg(rd).c_str(), imm);
}


RType::RType(const Elf32_Word& cmd) : InstructionType(cmd) {
    rd = get_rd(cmd);
    rs1 = get_rs1(cmd);
    rs2 = get_rs2(cmd);
    funct3 = get_funct3(cmd);
    funct7 = get_funct7(cmd);
    if ((funct7 & 1) == 0) {
        // R32I
        switch (funct3) {
            case 0b000: cmd_name = (funct7 ? "sub" : "add"); break;
            case 0b001: cmd_name = "sll"; break;
            case 0b010: cmd_name = "slt"; break;
            case 0b011: cmd_name = "sltu"; break;
            case 0b100: cmd_name = "xor"; break;
            case 0b101: cmd_name = (funct7 ? "srl" : "sra"); break;
            case 0b110: cmd_name = "or"; break;
            case 0b111: cmd_name = "and"; break;
        }
    } else {
        // R32M
        switch (funct3) {
            case 0b000: cmd_name = "mul";   break;
            case 0b001: cmd_name = "mulh";  break;
            case 0b010: cmd_name = "mulhsu";break;
            case 0b011: cmd_name = "mulhu"; break;
            case 0b100: cmd_name = "div";   break;
            case 0b101: cmd_name = "divu";  break;
            case 0b110: cmd_name = "rem";   break;
            case 0b111: cmd_name = "remu";  break;
        }
    }
}

void RType::print() {
    printf("%7s\t%s, %s, %s", cmd_name.c_str(), prettify_reg(rd).c_str(),
            prettify_reg(rs1).c_str(), prettify_reg(rs2).c_str());
}


SType::SType(const Elf32_Word& cmd) : InstructionType(cmd) {
    rs1 = get_rs1(cmd);
    rs2 = get_rs2(cmd);
    funct3 = get_funct3(cmd);
    imm = (get_blk(cmd, 7, 25) << 4) | get_blk(cmd, 4, 7);
    extend_sign(imm, 11);
    switch (funct3) {
        case 0b000: cmd_name = "sb"; break;
        case 0b001: cmd_name = "sh"; break;
        case 0b010: cmd_name = "sw"; break;
    }
}

void SType::print() {
    printf("%7s\t%s, %d(%s)", cmd_name.c_str(), prettify_reg(rs2).c_str(), imm,
            prettify_reg(rs1).c_str());
}

IType::IType(const Elf32_Word& cmd) : InstructionType(cmd) {
    rd = get_rd(cmd);
    rs1 = get_rs1(cmd);
    funct3 = get_funct3(cmd);
    imm = get_blk(cmd, 12, 20);
    extend_sign(imm, 11);
    if ((cmd & 0x7f) == EX_CTR) {
        is_exec = true;
        cmd_name = (imm & 1) ? "ebreak" : "ecall";
    } else if (cmd & 0x40) {
        is_load = true;
        cmd_name = "jalr";
    } else if (cmd & 0x10) {
        switch (funct3) {
            case 0b000: cmd_name = "addi"; break;
            case 0b010: cmd_name = "slti"; break;
            case 0b011: cmd_name = "sltiu"; break;
            case 0b100: cmd_name = "xori"; break;
            case 0b110: cmd_name = "ori"; break;
            case 0b111: cmd_name = "andi"; break;
            case 0b001: cmd_name = "slli"; break;
            case 0b101: cmd_name = (imm & 0x20) ? "srai" : "srli"; break;
        }
    } else {
        is_load = true;
        switch (funct3) {
            case 0b000: cmd_name = "lb"; break;
            case 0b001: cmd_name = "lh"; break;
            case 0b010: cmd_name = "lw"; break;
            case 0b100: cmd_name = "lbu"; break;
            case 0b101: cmd_name = "lhu"; break;
        }
    }
}

void IType::print() {
    if (is_exec) {
        printf("%7s", cmd_name.c_str());
    } else if (is_load) {
        printf("%7s\t%s, %d(%s)", cmd_name.c_str(), prettify_reg(rd).c_str(),
                imm, prettify_reg(rs1).c_str());
    } else {
        // ARITHI
        printf("%7s\t%s, %s, %d", cmd_name.c_str(), prettify_reg(rd).c_str(), 
                prettify_reg(rs1).c_str(), imm);
    }
}

JType::JType(const Elf32_Word& cmd) : InstructionType(cmd) {
    rd = get_rd(cmd);
    imm = (get_blk(cmd,  1, 31) << 20) |
          (get_blk(cmd, 10, 21) <<  1) |
          (get_blk(cmd,  1, 20) << 11) |
          (get_blk(cmd,  8, 12) << 12);
    cmd_name = "jal";
}

void JType::print() {
    printf("%7s\t%s, %x", cmd_name.c_str(), prettify_reg(rd).c_str(), imm);
}

BType::BType(const Elf32_Word& cmd) : InstructionType(cmd) {
    rs1 = get_rs1(cmd);
    rs2 = get_rs2(cmd);
    funct3 = get_funct3(cmd);
    imm = (get_blk(cmd, 1, 31) << 12) |
          (get_blk(cmd, 6, 25) <<  5) |
          (get_blk(cmd, 4,  8) <<  1) |
          (get_blk(cmd, 1,  7) << 11);
    switch (funct3) {
        case 0b000: cmd_name = "beq"; break;
        case 0b001: cmd_name = "bne"; break;
        case 0b100: cmd_name = "blt"; break;
        case 0b101: cmd_name = "bge"; break;
        case 0b110: cmd_name = "bltu"; break;
        case 0b111: cmd_name = "bgeu"; break;
    }
}

void BType::print() {
    printf("%7s\t%s, %s, %x", cmd_name.c_str(), prettify_reg(rs2).c_str(), 
            prettify_reg(rs1).c_str(), imm);
}


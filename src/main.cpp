#include "elfheader.h"
#include "elfsectiontable.h"
#include "elfsymbtable.h"
#include "typedefs.h"
#include "output.h"

#include <iostream>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <iterator>

void extend_sign(int& x, int sz) {
    for (; sz < 32; ++sz) {
        if (x & (1ll << sz)) {
            x |= (1ll << (sz + 1));
        }
    }
}

void print_code(
        const std::string& code,
        const std::string& symbols,
        std::vector<SymTableEntry> symtab,
        Elf32_Addr v_addr
    );

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


int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cout << "Usage: rv3 <elf_input_file_name> <output_file_name>\n";
        return 1;
    }
    std::ifstream fin((std::string) argv[1], std::ios::binary);

    Header header = parse_header(fin);
    std::vector<SectionTableEntry> sectionTable =
        parse_section_table(fin, header.e_shoff, header.e_shnum);
    SectionTableEntry &shstrHeader = sectionTable[header.e_shstrndx];

    auto find_tab_by_name = [&] (std::string name) {
        return std::find_if(
            sectionTable.begin(),
            sectionTable.end(),
            [&] (SectionTableEntry& el) {
                return get_section_name(fin, shstrHeader, el) == name;
            }
        ); 
    };
    std::vector<SectionTableEntry>::iterator symtabHeader = find_tab_by_name(".symtab");
    std::vector<SectionTableEntry>::iterator strtabHeader = find_tab_by_name(".strtab");
    std::vector<SectionTableEntry>::iterator textHeader = find_tab_by_name(".text");
    std::vector<SymTableEntry> symtab =
        parse_symtable(fin, symtabHeader->sh_offset,
                symtabHeader->sh_size / symtabHeader->sh_entsize);
    std::string symbols(strtabHeader->sh_size, 0);
    fin.seekg(strtabHeader->sh_offset);
    fin.read(&symbols.front(), symbols.size());

    std::string code(textHeader->sh_size, 0);
    fin.seekg(textHeader->sh_offset);
    fin.read(&code.front(), code.size());
    print_code(code, symbols, symtab, textHeader->sh_addr);
    printf("\n");
    print_symtable(symtab, symbols);
    return 0;
}

// Op codes
const unsigned char LUI    = 0b0110111;
const unsigned char AUIPC  = 0b0010111;
const unsigned char JAL    = 0b1101111;
const unsigned char JALR   = 0b1100111;
const unsigned char BRANCH = 0b1100011;
const unsigned char LOAD   = 0b0000011;
const unsigned char STORE  = 0b0100011;
const unsigned char ARITHI = 0b0010011;
const unsigned char ARITH  = 0b0110011;
const unsigned char FENCE  = 0b0001111;
const unsigned char EX_CTR = 0b1110011;

struct InstructionType {
    unsigned char opcode;
    InstructionType(const Elf32_Word& cmd) {
        opcode = cmd & 0x7f; 
    }

    unsigned char get_rd(Elf32_Word x) {
        return get_reg(x, 7);
    }

    unsigned char get_rs1(Elf32_Word x) {
        return get_reg(x, 15);
    }

    unsigned char get_rs2(Elf32_Word x) {
        return get_reg(x, 20);
    }

    unsigned char get_funct3(Elf32_Word x) {
        return get_blk(x, 3, 12);
    }

    unsigned char get_funct7(Elf32_Word x) {
        return get_blk(x, 7, 25);
    }

    unsigned char get_reg(Elf32_Word x, unsigned char pos) {
        return get_blk(x, 5, pos);
    }

    unsigned char get_cmd(Elf32_Word x) {
        return get_blk(x, 7, 0);
    }

    Elf32_Word get_blk(Elf32_Word x, unsigned char len, unsigned char pos) {
        return (x>>pos)&((1ll << len) - 1);
    }

    virtual void print() = 0;
    
    virtual ~InstructionType() = default;
};

struct UType : public InstructionType {
    std::string cmd_name;
    unsigned char rd;
    int imm;

    UType(const Elf32_Word& cmd) : InstructionType(cmd) {
        rd = get_rd(cmd);
        imm = get_blk(cmd, 20, 12) << 12;
        cmd_name = (get_cmd(cmd) == LUI ? "lui" : "auipc");
    }

    void print() {
        printf("%7s\t%s, %x", cmd_name.c_str(), prettify_reg(rd).c_str(), imm);
    }

    ~UType() = default;
};

struct RType : public InstructionType {
    std::string cmd_name;
    unsigned char rd;
    unsigned char rs1;
    unsigned char rs2;
    unsigned char funct3;
    unsigned char funct7;

    RType(const Elf32_Word& cmd) : InstructionType(cmd) {
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

    void print() {
        printf("%7s\t%s, %s, %s", cmd_name.c_str(), prettify_reg(rd).c_str(),
                prettify_reg(rs1).c_str(), prettify_reg(rs2).c_str());
    }

    ~RType() = default;
};

struct SType : public InstructionType {
    std::string cmd_name;
    unsigned char rs1;
    unsigned char rs2;
    unsigned char funct3;
    int imm;

    SType(const Elf32_Word& cmd) : InstructionType(cmd) {
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

    void print() {
        printf("%7s\t%s, %d(%s)", cmd_name.c_str(), prettify_reg(rs2).c_str(), imm,
                prettify_reg(rs1).c_str());
    }

    ~SType() = default;
};

struct IType : public InstructionType {
    std::string cmd_name;
    unsigned char rs1;
    unsigned char funct3;
    unsigned char rd;
    int imm;
    bool is_load = false;
    bool is_exec = false;

    IType(const Elf32_Word& cmd) : InstructionType(cmd) {
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

    void print() {
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

    ~IType() = default;
};

struct JType : public InstructionType {
    std::string cmd_name;
    unsigned char rd;
    Elf32_Word imm;

    JType(const Elf32_Word& cmd) : InstructionType(cmd) {
        rd = get_rd(cmd);
        imm = (get_blk(cmd,  1, 31) << 20) |
              (get_blk(cmd, 10, 21) <<  1) |
              (get_blk(cmd,  1, 20) << 11) |
              (get_blk(cmd,  8, 12) << 12);
        cmd_name = "jal";
    }

    void print() {
        printf("%7s\t%s, %x", cmd_name.c_str(), prettify_reg(rd).c_str(), imm);
    }

    ~JType() = default;
};

struct BType : public InstructionType {
    std::string cmd_name;
    unsigned char rs1;
    unsigned char rs2;
    unsigned char funct3;
    Elf32_Word imm;

    BType(const Elf32_Word& cmd) : InstructionType(cmd) {
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

    void print() {
        printf("%7s\t%s, %s, %x", cmd_name.c_str(), prettify_reg(rs2).c_str(), 
                prettify_reg(rs1).c_str(), imm);
    }

    ~BType() = default;
};


void print_code(
        const std::string& code,
        const std::string& symbols,
        std::vector<SymTableEntry> symtab,
        Elf32_Addr v_addr) {


    std::vector<SymTableEntry> functions;
    std::copy_if(symtab.begin(), symtab.end(), std::back_inserter(functions),
        [] (const SymTableEntry& el) {
            return type_by_info(el.st_info) == STT_FUNC;
        }
    );
    std::sort(functions.begin(), functions.end(), 
            [](const SymTableEntry& a, const SymTableEntry& b) {
                return a.st_value < b.st_value;
            }
    );

    auto it = functions.begin();

    for (size_t i = 0; i < code.size(); i += 4, v_addr += 4) {
        if (it->st_value == v_addr) {
            printf("%08x <%s>:\n", 
                    it->st_value, format_name(it->st_name, symbols).c_str());
            it++;
        }
        std::string cmd_str(code.begin() + i, code.begin() + i + 4);
        const char* buff = cmd_str.c_str();
        Elf32_Word cmd = *((Elf32_Word*)(buff));
        printf("   %05x:\t%08x\t", v_addr, cmd);

        unsigned char opcode = cmd & 0x7f;
        InstructionType* parsed_cmd = nullptr;
        if (opcode == LUI || opcode == AUIPC) {
            parsed_cmd = new UType(cmd);
        } else if (opcode == ARITH) {
            parsed_cmd = new RType(cmd);
        } else if (opcode == STORE) {
            parsed_cmd = new SType(cmd);
        } else if (opcode == EX_CTR || opcode == ARITHI || opcode == LOAD ||
                opcode == JALR) {
            parsed_cmd = new IType(cmd);
        } else if (opcode == BRANCH) {
            parsed_cmd = new BType(cmd);
        } else if (opcode == JAL) {
            parsed_cmd = new JType(cmd);
        }

        if (parsed_cmd != nullptr) {
            parsed_cmd->print();
            delete parsed_cmd;
        } else {
            printf("unknown_instruction");
        }
        printf("\n");

    }
    return;
}


#ifndef COMMAND_TYPES_GUARD
#define COMMAND_TYPES_GUARD

#include "typedefs.h"
#include <string>

struct InstructionType {
    unsigned char opcode;
    InstructionType(const Elf32_Word& cmd);

    unsigned char get_rd(Elf32_Word x);

    unsigned char get_rs1(Elf32_Word x);

    unsigned char get_rs2(Elf32_Word x);

    unsigned char get_funct3(Elf32_Word x);

    unsigned char get_funct7(Elf32_Word x);

    unsigned char get_reg(Elf32_Word x, unsigned char pos);

    unsigned char get_cmd(Elf32_Word x);

    Elf32_Word get_blk(Elf32_Word x, unsigned char len, unsigned char pos);

    virtual void print() = 0;
    
    virtual ~InstructionType() = default;
};

struct UType : public InstructionType {
    std::string cmd_name;
    unsigned char rd;
    int imm;

    UType(const Elf32_Word& cmd);

    void print();

    ~UType() = default;
};

struct RType : public InstructionType {
    std::string cmd_name;
    unsigned char rd;
    unsigned char rs1;
    unsigned char rs2;
    unsigned char funct3;
    unsigned char funct7;

    RType(const Elf32_Word& cmd);

    void print();

    ~RType() = default;
};

struct SType : public InstructionType {
    std::string cmd_name;
    unsigned char rs1;
    unsigned char rs2;
    unsigned char funct3;
    int imm;

    SType(const Elf32_Word& cmd);

    void print();

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

    IType(const Elf32_Word& cmd);

    void print(); 

    ~IType() = default;
};

struct JType : public InstructionType {
    std::string cmd_name;
    unsigned char rd;
    Elf32_Word imm;

    JType(const Elf32_Word& cmd);

    void print();

    ~JType() = default;
};

struct BType : public InstructionType {
    std::string cmd_name;
    unsigned char rs1;
    unsigned char rs2;
    unsigned char funct3;
    Elf32_Word imm;

    BType(const Elf32_Word& cmd);

    void print();

    ~BType() = default;
};

#endif

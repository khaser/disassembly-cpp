#include "elfsymbtable.h"
#include "command_types.h"
#include "output_symbtable.h"

#include <string>
#include <algorithm>
#include <vector>

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
            printf("%08x   <%s>:\n", 
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
            parsed_cmd = new UType(cmd, v_addr, functions, symbols);
        } else if (opcode == ARITH) {
            parsed_cmd = new RType(cmd, v_addr, functions, symbols);
        } else if (opcode == STORE) {
            parsed_cmd = new SType(cmd, v_addr, functions, symbols);
        } else if (opcode == EX_CTR || opcode == ARITHI || opcode == LOAD ||
                opcode == JALR) {
            parsed_cmd = new IType(cmd, v_addr, functions, symbols);
        } else if (opcode == BRANCH) {
            parsed_cmd = new BType(cmd, v_addr, functions, symbols);
        } else if (opcode == JAL) {
            parsed_cmd = new JType(cmd, v_addr, functions, symbols);
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

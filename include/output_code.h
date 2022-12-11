#ifndef OUTPUTCODE_GUARDS
#define OUTPUTCODE_GUARDS

#include "typedefs.h"
#include "elfsymbtable.h"

#include <string>
#include <vector>

void print_code(
        const std::string& code,
        const std::string& symbols,
        std::vector<SymTableEntry> symtab,
        Elf32_Addr v_addr
    );


#endif

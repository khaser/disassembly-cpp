#ifndef ELFSYMBTABLE_GUARDS
#define ELFSYMBTABLE_GUARDS

#include "typedefs.h"

#include <vector>
#include <istream>

struct SymTableEntry {
    Elf32_Word    st_name;
    Elf32_Addr    st_value;
    Elf32_Word    st_size;
    unsigned char st_info;    
    unsigned char st_other;
    Elf32_Half    st_shndx;   
};

std::vector<SymTableEntry> parse_symtable(
        std::istream&,
        Elf32_Addr offset,
        unsigned int entries);


#endif

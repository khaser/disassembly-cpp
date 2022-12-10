#ifndef ELFSECTIONTABLE_GUARDS
#define ELFSECTIONTABLE_GUARDS

#include "typedefs.h"

#include <vector>
#include <istream>

struct SectionTableEntry {
    Elf32_Word    sh_name;    
    Elf32_Word    sh_type;    
    Elf32_Word    sh_flags;   
    Elf32_Addr    sh_addr;    
    Elf32_Off     sh_offset;  
    Elf32_Word    sh_size;    
    Elf32_Word    sh_link;    
    Elf32_Word    sh_info;    
    Elf32_Word    sh_addralign; 
    Elf32_Word    sh_entsize;   
};


std::vector<SectionTableEntry> parse_section_table 
    (std::istream &ss, Elf32_Addr addr, unsigned int entries);

std::string get_section_name (
        std::istream &ss, 
        const SectionTableEntry& shstrEntry,
        const SectionTableEntry& section);

#endif

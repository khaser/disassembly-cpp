#ifndef ELFHEADER_GUARDS
#define ELFHEADER_GUARDS

#include "typedefs.h"

#include <istream>

struct Header {
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half    e_type;         
    Elf32_Half    e_machine;      
    Elf32_Word    e_version;      
    Elf32_Addr    e_entry;        
    Elf32_Off     e_phoff;        
    Elf32_Off     e_shoff;        
    Elf32_Word    e_flags;        
    Elf32_Half    e_ehsize;       
    Elf32_Half    e_phentsize;    
    Elf32_Half    e_phnum;        
    Elf32_Half    e_shentsize;    
    Elf32_Half    e_shnum;        
    Elf32_Half    e_shstrndx;     
};

Header parse_header(std::istream &ss);

#endif

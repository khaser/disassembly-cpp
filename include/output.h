#ifndef OUTPUT_GUARDS
#define OUTPUT_GUARDS

#include "typedefs.h"
#include "elfsymbtable.h"

#include <vector>
#include <string>

void print_symtable(const std::vector<SymTableEntry>&, const std::string&);

std::string format_bind(const unsigned char& info);

std::string format_type(const unsigned char& info);

std::string format_index(const Elf32_Half& idx);

std::string format_vis(const unsigned char& st_other);

std::string format_name(const Elf32_Word& name_offset, const std::string& symbols); 

inline unsigned char type_by_info(const unsigned char& info) {
    return (info&0xf);
}

inline unsigned char bind_by_info(const unsigned char& info) {
    return (info>>4);
}

#endif

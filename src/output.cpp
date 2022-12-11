#include "output.h"
#include "elfsymbtable.h"
#include "typedefs.h"

#include <stdexcept>
#include <vector>
#include <string>
#include <tuple>
#include <algorithm>
#include <exception>

std::string format_bind(const unsigned char& info) {
    unsigned char bind = (info>>4);
    if (bind == STB_LOCAL)
        return "LOCAL";
    else if (bind == STB_GLOBAL)
        return "GLOBAL";
    else if (bind == STB_WEAK) 
        return "WEAK";
    else if (bind == STB_LOPROC) 
        return "LOPROC";
    else if (bind == STB_HIPROC) 
        return "HIPROC";
    throw std::invalid_argument("Undefined bind value");
}

std::string format_type(const unsigned char& info) {
    unsigned char type = (info&0xf);
    if (type == STT_NOTYPE)
        return "NOTYPE";
    else if (type == STT_OBJECT)
        return "OBJECT";
    else if (type == STT_FUNC) 
        return "FUNC";
    else if (type == STT_SECTION)
        return "SECTION";
    else if (type == STT_FILE)
        return "FILE";
    else if (type == STT_LOPROC)
        return "LOPROC";
    else if (type == STT_HIPROC)
        return "HIPROC";
    throw std::invalid_argument("Undefined type value");
}

std::string format_index(const Elf32_Half& idx) {
    if (idx == SHN_ABS) 
        return "ABS";
    else if (idx == SHN_UNDEF) 
        return "UNDEF";
    else 
        return std::to_string(idx);
}

std::string format_vis(const unsigned char& st_other) {
    int vis = (st_other&0x7);
    if (vis == STV_DEFAULT) 
        return "DEFAULT";
    else if (vis == STV_INTERNAL)
        return "INTERNAL";
    else if (vis == STV_HIDDEN)
        return "HIDDEN";
    else if (vis == STV_EXPORTED)
        return "EXPORTED";
    else if (vis == STV_SINGLETON)
        return "SINGLETON";
    else if (vis == STV_ELIMINATE)
        return "ELIMINATE";
    else if (vis == STV_NUM)
        return "NUM";
    throw std::invalid_argument("Undefined visibility value");
}

std::string format_name(const Elf32_Word& name_offset, const std::string& symbols) {
    return 
        std::string (
            symbols.begin() + name_offset,
            find(symbols.begin() + name_offset, symbols.end(), '\0')
        );
}

void print_symtable(
        const std::vector<SymTableEntry>& symtable,
        const std::string& symbols) {
    printf("Symbol Value              Size Type     Bind     Vis       Index Name\n");
    for (size_t i = 0; i < symtable.size(); ++i) {
        const SymTableEntry &el = symtable[i];
        std::string bind  = format_bind(el.st_info);
        std::string type  = format_type(el.st_info);
        std::string index(format_index(el.st_shndx));
        std::string name = format_name(el.st_name, symbols);
        std::string vis(format_vis(el.st_other));
        printf("[%4zu] 0x%-15X %5i %-8s %-8s %-8s %6s %s\n", 
                i, el.st_value, el.st_size, type.c_str(), bind.c_str(),
                vis.c_str(), index.c_str(), name.c_str());
    }
}


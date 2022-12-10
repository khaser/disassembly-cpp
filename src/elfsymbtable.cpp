#include "elfsymbtable.h"
#include "elfsectiontable.h"
#include "typedefs.h"

#include <istream>
#include <vector>

std::vector<SymTableEntry> parse_symtable(
        std::istream &ss,
        Elf32_Addr offset,
        unsigned int entries) {
    ss.seekg(offset);
    std::vector<SymTableEntry> res(entries);
    for (SymTableEntry &i : res) {
        ss.read((char*) &i, sizeof(SymTableEntry));
    }
    return res;
}


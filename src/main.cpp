#include "elfheader.h"
#include "elfsectiontable.h"
#include "elfsymbtable.h"
#include "typedefs.h"
#include "output_symbtable.h"
#include "output_code.h"

#include <iostream>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <iterator>
#include <cstdio>

int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cout << "Usage: rv3 <elf_input_file_name> <output_file_name>\n";
        return 1;
    }
    std::ifstream fin((std::string) argv[1], std::ios::binary);
    freopen(argv[2], "w", stdout);

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

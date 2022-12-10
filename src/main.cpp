#include "elfheader.h"
#include "elfsectiontable.h"
#include "elfsymbtable.h"
#include "typedefs.h"
#include "output.h"

#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <string>
#include <iterator>

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cout << "Usage: rv3 <elf_input_file_name> <output_file_name>\n";
        return 1;
    }

    std::ifstream fin((std::string) argv[1], std::ios::binary);

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

    std::vector<SymTableEntry> symtab =
        parse_symtable(fin, symtabHeader->sh_offset,
                symtabHeader->sh_size / symtabHeader->sh_entsize);

    std::string symbols(strtabHeader->sh_size, 0);
    fin >> symbols;

    print_symtable(symtab, symbols);

    // --
    std::cout << header.e_shnum << ' ' << header.e_shstrndx << '\n';
    for (auto i : sectionTable) {
        std::cout << get_section_name(fin, shstrHeader, i) << ' ';
    }
    // --
    return 0;
}

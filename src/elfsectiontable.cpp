#include "elfsectiontable.h"
#include "typedefs.h"

#include <vector>
#include <istream>

std::vector<SectionTableEntry> parse_section_table (
     std::istream &ss,
     Elf32_Addr addr,
     unsigned int entries
 ) {
    ss.seekg(addr);
    std::vector<SectionTableEntry> res(entries);
    for (SectionTableEntry& i : res) {
        ss.read((char*)&i, sizeof(SectionTableEntry));
    }
    return res;
}

std::string get_section_name (
        std::istream &ss, 
        const SectionTableEntry& shstrEntry,
        const SectionTableEntry& section)
{
    ss.seekg(shstrEntry.sh_offset + section.sh_name);
    char res[MAX_NAME];
    ss.getline(res, MAX_NAME, '\0');
    return (std::string) res;
};



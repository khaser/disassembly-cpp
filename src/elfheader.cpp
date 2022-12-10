#include "elfheader.h"
#include "elfsectiontable.h"

#include <exception>

Header parse_header(std::istream &ss) {

    const unsigned int expected_magic = 0x464C457F;
    const unsigned char expected_arch = 1;
    const unsigned char expected_endian = 1;
    const unsigned char SysV_ABI_code = 0x00;
    const unsigned short RiscV_ISA_code = 0x00F3;
    const unsigned short header_expected_size = 52;

    Header header;

    ss.read((char*)&header, sizeof(header));
    uint32_t magic = *(uint32_t*)header.e_ident;
    if (magic != expected_magic) {
        throw std::invalid_argument("Invalid ELF magic bits");
    }
    if (header.e_ident[5] != expected_arch) {
        throw std::invalid_argument("Unsupported architecture. 32bit only");
    }
    if (header.e_ident[6] != expected_endian) {
        throw std::invalid_argument("Big endian is no supported");
    }
    if (header.e_ident[7] != SysV_ABI_code) {
        throw std::invalid_argument("Unsupported ABI. System V only");
    }
    if (header.e_machine != RiscV_ISA_code) {
        throw std::invalid_argument("Unsupported ISA. RISC-V only");
    }
    if (header.e_ehsize != header_expected_size) {
        throw std::invalid_argument("Illegal elf header size");
    }
    if (header.e_shentsize != sizeof(SectionTableEntry)) {
        throw std::invalid_argument("Unexpected section header table entry size");
    }
    return header;
};


#ifndef TYPEDEFS_GUARD
#define TYPEDEFS_GUARD

typedef unsigned int   Elf32_Addr;
typedef unsigned short Elf32_Half;
typedef unsigned int   Elf32_Off;
typedef int            Elf32_Sword;
typedef unsigned int   Elf32_Word;

const unsigned short EI_NIDENT = 16;

const unsigned int MAX_NAME = 256;

const unsigned short SHN_ABS = 0xfff1;

const unsigned short SHN_UNDEF = 0;

#define STB_LOCAL      0
#define STB_GLOBAL     1
#define STB_WEAK       2
#define STB_NUM        3

#define STB_LOPROC    13
#define STB_HIPROC    15

#define STT_NOTYPE     0
#define STT_OBJECT     1
#define STT_FUNC       2
#define STT_SECTION    3
#define STT_FILE       4
#define STT_COMMON     5
#define STT_TLS        6
#define STT_NUM        7

#define STT_LOPROC    13
#define STT_HIPROC    15


#define STV_DEFAULT    0
#define STV_INTERNAL   1
#define STV_HIDDEN     2
#define STV_PROTECTED  3
#define STV_EXPORTED   4
#define STV_SINGLETON  5
#define STV_ELIMINATE  6

#define STV_NUM        7


#endif

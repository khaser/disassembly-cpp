#ifndef OUTPUT_GUARDS
#define OUTPUT_GUARDS

#include "typedefs.h"
#include "elfsymbtable.h"

#include <vector>
#include <string>

void print_symtable(const std::vector<SymTableEntry>&, std::string symbols);

#endif

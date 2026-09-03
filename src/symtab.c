#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "symtab.h"

void symtab_initialize(SymbolTable *symtab) {
    symtab->sym_count = 0;
    symtab->curr_offset = -4;
}

int symtab_add(SymbolTable *symtab, char *name) {
    if (symtab->sym_count > 255) {
        printf("Error: Symbol Table overflowed.");
        exit(1);
    }

    int offset = symtab->curr_offset;
    strncpy(symtab->symbols[symtab->sym_count].name, name, 63);
    symtab->symbols[symtab->sym_count].name[63] = '\0';
    symtab->symbols[symtab->sym_count].offset = offset;
    symtab->curr_offset -= 4;
    symtab->sym_count++;

    return offset;
}

int symtab_lookup(SymbolTable *symtab, char* name) {
    for (int i = symtab->sym_count-1; i >= 0; i--) {
        if (strcmp(symtab->symbols[i].name, name) == 0) {
            return symtab->symbols[i].offset;
        }
    }
    return -1;
}
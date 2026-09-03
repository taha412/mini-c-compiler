#ifndef SYMTAB_H
#define SYMTAB_H

typedef struct Symbol {
    char name[64];
    int offset;
} Symbol;

typedef struct SymbolTable {
    Symbol symbols[256]; // shortcut by implemented max 256 symbols per table (meets C99 standard). When changed make sure to update size check in symtab_add
    int sym_count;
    int curr_offset;
} SymbolTable;

void symtab_initialize(SymbolTable *symtab);
int symtab_add(SymbolTable *symtab, char *name);
int symtab_lookup(SymbolTable *symtab, char* name);


#endif
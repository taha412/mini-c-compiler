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
    int scope_count;
    int scope_clause_count;
} SymbolTable;

typedef enum GLOBAL_TYPE {
    GLOB_FUNC
} GLOBAL_TYPE;

typedef struct GlobalSymbol {
    GLOBAL_TYPE type;
    char name[64];
    int num_params;
    GlobalSymbol *next;
} GlobalSymbol;

typedef struct GlobalEnv {

} GlobalEnv;

void symtab_initialize(SymbolTable *symtab);
int symtab_add(SymbolTable *symtab, char *name);
int symtab_lookup(SymbolTable *symtab, char* name);
int symtab_lookup_in_scope(SymbolTable *symtab, char* name);


#endif
#ifndef SYMTAB_H
#define SYMTAB_H

#include "parser.h"

typedef struct Symbol {
    char name[64];
    DataType data_type;
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
    int para_count;
    DataType return_type;
    struct GlobalSymbol *next;
} GlobalSymbol;

// tracks functions that are declared and their parameters
typedef struct GlobalEnv {
    GlobalSymbol *head;
} GlobalEnv;

void symtab_initialize(SymbolTable *symtab);
int symtab_add(SymbolTable *symtab, char *name, DataType data_type);
int symtab_add_temp(SymbolTable *symtab); // reserves slot in memory without adding a symbol
Symbol *symtab_lookup(SymbolTable *symtab, char* name);
int symtab_lookup_in_scope(SymbolTable *symtab, char* name);

void globenv_initialize(GlobalEnv *globenv);
GlobalSymbol *globenv_lookup(GlobalEnv *globenv, char *name);
void globenv_add_func(GlobalEnv *globenv, Function *fnctn);


#endif
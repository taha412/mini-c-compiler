#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "symtab.h"

void symtab_initialize(SymbolTable *symtab) {
    symtab->sym_count = 0;
    symtab->curr_offset = -4;
    symtab->scope_count = 0;
    symtab->scope_clause_count = -1; // stores clause count of current scope for continue and break logic
}

int symtab_add(SymbolTable *symtab, char *name, DataType data_type) {
    if (symtab->sym_count > 255) {
        printf("Error: Symbol Table overflowed.");
        exit(1);
    }

    int offset = symtab->curr_offset;

    strncpy(symtab->symbols[symtab->sym_count].name, name, 63);
    symtab->symbols[symtab->sym_count].name[63] = '\0';

    symtab->symbols[symtab->sym_count].offset = offset;
    symtab->symbols[symtab->sym_count].data_type = data_type;
    symtab->curr_offset -= 4;
    symtab->sym_count++;

    return offset;
}

int symtab_add_temp(SymbolTable *symtab) { // reserves slot in memory without adding a symbol
    int offset = symtab->curr_offset;
    symtab->curr_offset -= 4;
    return offset;
}

Symbol *symtab_lookup(SymbolTable *symtab, char* name) {
    for (int i = symtab->sym_count-1; i >= 0; i--) {
        if (strcmp(symtab->symbols[i].name, name) == 0) {
            return &(symtab->symbols[i]);
        }
    }
    return NULL;
}

int symtab_lookup_in_scope(SymbolTable *symtab, char* name) {
    for (int i = symtab->sym_count-1; i >= symtab->scope_count; i--) {
        if (strcmp(symtab->symbols[i].name, name) == 0) {
            return symtab->symbols[i].offset;
        }
    }
    return -1;
}

void globenv_initialize(GlobalEnv *globenv) {
    globenv->head = NULL;
}

GlobalSymbol *globenv_lookup(GlobalEnv *globenv, char *name) {
    for (GlobalSymbol *gs = globenv->head; gs != NULL; gs = gs->next) {
        if (strcmp(gs->name, name) == 0) {
            return gs;
        }
    }
    return NULL;
}

void globenv_add_func(GlobalEnv *globenv, Function *fnctn) {
    GlobalSymbol *gs = calloc(1, sizeof(GlobalSymbol));
    gs->type = GLOB_FUNC;

    strncpy(gs->name, fnctn->name, 63);
    gs->name[63] = '\0';

    gs->para_count = fnctn->para_count;
    gs->return_type = fnctn->return_type;

    gs->next = globenv->head;
    globenv->head = gs;
}
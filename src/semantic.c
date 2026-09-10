#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "semantic.h"
#include "symtab.h"
#include "parser.h"

int largest_offset = 0;
int curr_clause_count = 0;

int get_clause_count() {
    return curr_clause_count++;
}

static void resolve_block_item(BlockItem *bi, SymbolTable *symtab, GlobalEnv *ge);

static void update_largest_offset(int offset) {
    if (-offset > largest_offset) { // offset counts down into negatives, make positive
        largest_offset = -offset;
    }
    return;
}

static void resolve_expression(Expression *expr, SymbolTable *symtab, GlobalEnv *ge) {    
    if (expr->type == EXPR_UNOP) {
        switch (expr->un_op) {
            case OP_NEG:
                resolve_expression(expr->lterm, symtab, ge);
                if (expr->lterm->data_type == DATA_INT) {
                    expr->data_type = DATA_INT;
                }
                break;
            case OP_COMPL:
                resolve_expression(expr->lterm, symtab, ge);
                if (expr->lterm->data_type == DATA_INT) {
                    expr->data_type = DATA_INT;
                }
                break;
            case OP_NOT:
                resolve_expression(expr->lterm, symtab, ge);
                if (expr->lterm->data_type == DATA_INT) {
                    expr->data_type = DATA_INT;
                }
                break;
            default:
                printf("Error: Unknown unary operator.\n");
                exit(1);
        }
    }

    else if (expr->type == EXPR_BINOP) {
        switch (expr->bin_op) {
            case BIN_NEG:
                resolve_expression(expr->rterm, symtab, ge);
                resolve_expression(expr->lterm, symtab, ge);
                if (expr->lterm->data_type == DATA_INT && expr->rterm->data_type == DATA_INT) {
                    expr->data_type = DATA_INT;
                }
                break;
            case BIN_ADD:
                resolve_expression(expr->rterm, symtab, ge);
                resolve_expression(expr->lterm, symtab, ge);
                if (expr->lterm->data_type == DATA_INT && expr->rterm->data_type == DATA_INT) {
                    expr->data_type = DATA_INT;
                }
                break;
            case BIN_MULTIPLY:
                resolve_expression(expr->rterm, symtab, ge);
                resolve_expression(expr->lterm, symtab, ge);
                if (expr->lterm->data_type == DATA_INT && expr->rterm->data_type == DATA_INT) {
                    expr->data_type = DATA_INT;
                }
                break;
            case BIN_DIVIDE:
                resolve_expression(expr->rterm, symtab, ge);
                resolve_expression(expr->lterm, symtab, ge);
                if (expr->lterm->data_type == DATA_INT && expr->rterm->data_type == DATA_INT) {
                    expr->data_type = DATA_INT;
                }
                break;
            case BIN_MOD:
                resolve_expression(expr->rterm, symtab, ge);
                resolve_expression(expr->lterm, symtab, ge);
                if (expr->lterm->data_type == DATA_INT && expr->rterm->data_type == DATA_INT) {
                    expr->data_type = DATA_INT;
                }
                break;
            case BIN_EQ:
                resolve_expression(expr->rterm, symtab, ge);
                resolve_expression(expr->lterm, symtab, ge);
                if (expr->lterm->data_type == DATA_INT && expr->rterm->data_type == DATA_INT) {
                    expr->data_type = DATA_INT;
                }
                break;
            case BIN_NEQ:
                resolve_expression(expr->rterm, symtab, ge);
                resolve_expression(expr->lterm, symtab, ge);
                if (expr->lterm->data_type == DATA_INT && expr->rterm->data_type == DATA_INT) {
                    expr->data_type = DATA_INT;
                }
                break;
            case BIN_LT:
                resolve_expression(expr->lterm, symtab, ge);
                resolve_expression(expr->rterm, symtab, ge);
                if (expr->lterm->data_type == DATA_INT && expr->rterm->data_type == DATA_INT) {
                    expr->data_type = DATA_INT;
                }
                break;
            case BIN_LTE:
                resolve_expression(expr->lterm, symtab, ge);
                resolve_expression(expr->rterm, symtab, ge);
                if (expr->lterm->data_type == DATA_INT && expr->rterm->data_type == DATA_INT) {
                    expr->data_type = DATA_INT;
                }
                break;
            case BIN_GT:
                resolve_expression(expr->lterm, symtab, ge);
                resolve_expression(expr->rterm, symtab, ge);
                if (expr->lterm->data_type == DATA_INT && expr->rterm->data_type == DATA_INT) {
                    expr->data_type = DATA_INT;
                }
                break;
            case BIN_GTE:
                resolve_expression(expr->lterm, symtab, ge);
                resolve_expression(expr->rterm, symtab, ge);
                if (expr->lterm->data_type == DATA_INT && expr->rterm->data_type == DATA_INT) {
                    expr->data_type = DATA_INT;
                }
                break;
            case BIN_AND:
                expr->clause_count = get_clause_count();
                resolve_expression(expr->lterm, symtab, ge);
                resolve_expression(expr->rterm, symtab, ge);
                if (expr->lterm->data_type == DATA_INT && expr->rterm->data_type == DATA_INT) {
                    expr->data_type = DATA_INT;
                }
                break;
            case BIN_OR:
                expr->clause_count = get_clause_count();
                resolve_expression(expr->lterm, symtab, ge);
                resolve_expression(expr->rterm, symtab, ge);
                if (expr->lterm->data_type == DATA_INT && expr->rterm->data_type == DATA_INT) {
                    expr->data_type = DATA_INT;
                }
                break;
            default:
                printf("Error: Unknown binary operator.\n");
                exit(1);
        }
    }

    else if (expr->type == EXPR_ASS) {
        Symbol *sym = symtab_lookup(symtab, expr->lterm->text);
        if (sym == NULL) {
            printf("Error: Variable '%s' is not defined in this scope\n", expr->lterm->text);
            exit(1);
        }
        expr->lterm->resolved_offset = sym->offset;
        resolve_expression(expr->rterm, symtab, ge);
    }

    else if (expr->type == EXPR_VAR) {
        Symbol *sym = symtab_lookup(symtab, expr->text);
        if (sym == NULL) {
            printf("Error: Variable '%s' is not defined in this scope\n", expr->text);
            exit(1);
        }
        expr->resolved_offset = sym->offset;
        expr->data_type = sym->data_type;
    }

    else if (expr->type == EXPR_TERNARY) {
        expr->clause_count = get_clause_count();
        resolve_expression(expr->term_cond, symtab, ge);
        resolve_expression(expr->term_one, symtab, ge);
        resolve_expression(expr->term_two, symtab, ge);
    }

    else if (expr->type == EXPR_CALL) {
        GlobalSymbol *gs = globenv_lookup(ge, expr->text);

        if (gs == NULL) {
            printf("Error: Function %s is not declared\n", expr->text);
            exit(1);
        }

        if (gs->para_count != expr->arg_count) {
            printf("Error: Function call %s expects %d arguments but got %d\n", expr->text, gs->para_count, expr->arg_count);
            exit(1);
        }

        //TODO Add type checking later
        for (Argument *arg = expr->args; arg != NULL; arg = arg->next) {
            resolve_expression(arg->expr, symtab, ge);
            
            int offset = symtab_add_temp(symtab);
            arg->resolved_offset = offset;
            update_largest_offset(offset);
        }

        expr->data_type = gs->return_type;
    }

    return;
}

// symtab pass by value instead of pass by reference so local variables created in an inner scope do not exist in the outer scope
static void resolve_statement(Statement *stmt, SymbolTable symtab, GlobalEnv *ge) {
    symtab.scope_count = symtab.sym_count; // new scope for local variables
    if (stmt->type == STMT_RETURN) {
        resolve_expression(stmt->expr, &symtab, ge);
        return;
    }

    if (stmt->type == STMT_BLOCK) {
        BlockItem *curr = stmt->block_head;
        while (curr != NULL) {
            resolve_block_item(curr, &symtab, ge);
            if (curr->type == BLCKITEM_STMT && curr->stmt->type == STMT_RETURN) {
                if (curr->next != NULL) {
                    printf("Warning: Code will never be reached\n");
                    curr->next = NULL;
                }
                break;
            }
            curr = curr->next;
        }
    }

    else if (stmt->type == STMT_EXPR) {
        resolve_expression(stmt->expr, &symtab, ge);
    }

    else if (stmt->type == STMT_COND) {
        stmt->clause_count = get_clause_count();
        resolve_expression(stmt->expr, &symtab, ge);
        resolve_statement(stmt->if_stmt, symtab, ge);

        if (stmt->else_stmt != NULL) {
            resolve_statement(stmt->else_stmt, symtab, ge);
        }   
    }

    else if (stmt->type == STMT_FOR) {
        stmt->clause_count = get_clause_count();
        symtab.scope_clause_count = stmt->clause_count;
        if (stmt->init != NULL) {
            resolve_block_item(stmt->init, &symtab, ge);
        }

        if (stmt->cond != NULL) {
            resolve_expression(stmt->cond, &symtab, ge);
        }

        resolve_statement(stmt->loop_stmt, symtab, ge);

        if (stmt->post != NULL) {
            resolve_expression(stmt->post, &symtab, ge);
        }
    }

    else if (stmt->type == STMT_WHILE) {
        stmt->clause_count = get_clause_count();
        symtab.scope_clause_count = stmt->clause_count;
        resolve_expression(stmt->expr, &symtab, ge);
        resolve_statement(stmt->loop_stmt, symtab, ge);
    }

    else if (stmt->type == STMT_DO_WHILE) {
        stmt->clause_count = get_clause_count();
        symtab.scope_clause_count = stmt->clause_count;
        resolve_statement(stmt->loop_stmt, symtab, ge);
        resolve_expression(stmt->expr, &symtab, ge);
        return;
    }

    else if (stmt->type == STMT_CONT) {
        if (symtab.scope_clause_count == -1) {
            printf("Error: Can not use continue outside of a loop\n");
            exit(1);
        }
        stmt->clause_count = symtab.scope_clause_count;
        return;
    }

    else if (stmt->type == STMT_BREAK) {
        if (symtab.scope_clause_count == -1) {
            printf("Error: Can not use break outside of a loop\n");
            exit(1);
        }
        stmt->clause_count = symtab.scope_clause_count;
        return;
    }

    else if (stmt->type == STMT_NULL) {
        return;
    }

    return;
}

static void resolve_declaration(Declaration *decl, SymbolTable *symtab, GlobalEnv *ge) {
    if (decl->data_type == DATA_INT) {
        if (symtab_lookup_in_scope(symtab, decl->name) != -1) {
            printf("Error: variable with name %s is already defined\n", decl->name);
            exit(1);
        }

        if (decl->expr != NULL) {
            resolve_expression(decl->expr, symtab, ge);
        }
        int offset = symtab_add(symtab, decl->name, decl->data_type);
        decl->resolved_offset = offset;
        update_largest_offset(offset);
    }
    return;
}

static void resolve_block_item(BlockItem *bi, SymbolTable *symtab, GlobalEnv *ge) {
    if (bi->type == BLCKITEM_STMT) {
        resolve_statement(bi->stmt, *symtab, ge);
    }

    else if (bi->type == BLCKITEM_DECL) {
        resolve_declaration(bi->decl, symtab, ge);
    }

    return;
}

static void resolve_function(Function *fnctn, GlobalEnv *ge) {
    largest_offset = 0;
    SymbolTable symtab;
    symtab_initialize(&symtab);

    for (Parameter *p = fnctn->para; p != NULL; p = p->next) {
        if (symtab_lookup_in_scope(&symtab, p->name) != -1) {
            printf("Error: Duplicate parameter name %s\n", p->name);
            exit(1);
        }

        int offset = symtab_add(&symtab, p->name, p->data_type);
        p->resolved_offset = offset;
        update_largest_offset(offset);
    }

    resolve_statement(fnctn->stmt, symtab, ge);

    // round to next highest multiple of 16
    fnctn->frame_size = (largest_offset + 15) & ~15;
}

static void resolve_top_lvl_item(TopLevelItem *tpi, GlobalEnv *ge) {
    if (tpi->type == TOPLVL_FNCTN) {
        if (tpi->fnctn->stmt != NULL) { // TODO: Maybe move null check to resolve_function
            resolve_function(tpi->fnctn, ge);
        }
    }
}

// look away
// TODO: improve from O(n^2)   :(
static void verify_func_declarations(TopLevelItem *tli) {
    TopLevelItem *i1 = tli;
    TopLevelItem *i2 = tli;
    while (i1 != NULL) {
        if (i1->type == TOPLVL_FNCTN) {
            i2 = i1->next;
            while (i2 != NULL) {
                if (i2->type == TOPLVL_FNCTN) {
                    if (strcmp(i1->fnctn->name, i2->fnctn->name) == 0) {
                        if (i1->fnctn->stmt != NULL && i2->fnctn->stmt != NULL) {
                            printf("Error: Function '%s' has multiple definitions\n", i1->fnctn->name);
                            exit(1);
                        }

                        if (i1->fnctn->para_count != i2->fnctn->para_count) {
                            printf("Error: Function '%s' declaration parameters do not match definition\n", i1->fnctn->name);
                            exit(1);
                        }

                        if (i1->fnctn->return_type != i2->fnctn->return_type) {
                            printf("Error: Function '%s' return types do not match between definitions\n", i1->fnctn->name);
                            exit(1);
                        }

                        Parameter *p1 = i1->fnctn->para;
                        Parameter *p2 = i2->fnctn->para;
                        while (p1 != NULL && p2 != NULL) {
                            if (p1->data_type != p2->data_type) {
                                printf("Error: Function '%s' declaration parameters do not match definition\n", i1->fnctn->name);
                                exit(1);
                            }
                            p1 = p1->next;
                            p2 = p2->next;
                        }
                    }
                }
                i2 = i2->next;
            }
        }
        i1 = i1->next;
    }
}

void resolve_code(Program *prog) {
    verify_func_declarations(prog->top);
    GlobalEnv ge;
    globenv_initialize(&ge);
    TopLevelItem *curr = prog->top;


    while (curr != NULL) {
        if (curr->type == TOPLVL_FNCTN) {
            globenv_add_func(&ge, curr->fnctn);
            if (curr->fnctn->stmt != NULL) {
                resolve_top_lvl_item(curr, &ge);
            }
        }
        curr = curr->next;
    }
}
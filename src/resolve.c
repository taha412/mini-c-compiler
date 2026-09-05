#include <stdio.h>
#include <stdlib.h>

#include "resolve.h"
#include "symtab.h"
#include "parser.h"

int largest_offset = 0;

static void resolve_block_item(BlockItem *bi, SymbolTable *symtab);

static void update_largest_offset(int offset) {
    if (-offset > largest_offset) { // offset counts down into negatives, make positive
        largest_offset = -offset;
    }
    return;
}

static void resolve_expression(Expression *expr, SymbolTable *symtab) {    
    if (expr->type == EXPR_UNOP) {
        switch (expr->un_op) {
            case OP_NEG:
                resolve_expression(expr->lterm, symtab);
                break;
            case OP_COMPL:
                resolve_expression(expr->lterm, symtab);
                break;
            case OP_NOT:
                resolve_expression(expr->lterm, symtab);
                break;
            default:
                printf("Error: Unknown unary operator.\n");
                exit(1);
        }
    }

    else if (expr->type == EXPR_BINOP) {
        switch (expr->bin_op) {
            case BIN_NEG:
                resolve_expression(expr->rterm, symtab);
                resolve_expression(expr->lterm, symtab);
                break;
            case BIN_ADD:
                resolve_expression(expr->rterm, symtab);
                resolve_expression(expr->lterm, symtab);
                break;
            case BIN_MULTIPLY:
                resolve_expression(expr->rterm, symtab);
                resolve_expression(expr->lterm, symtab);
                break;
            case BIN_DIVIDE:
                resolve_expression(expr->rterm, symtab);
                resolve_expression(expr->lterm, symtab);
                break;
            case BIN_MOD:
                resolve_expression(expr->rterm, symtab);
                resolve_expression(expr->lterm, symtab);
                break;
            case BIN_EQ:
                resolve_expression(expr->rterm, symtab);
                resolve_expression(expr->lterm, symtab);
                break;
            case BIN_NEQ:
                resolve_expression(expr->rterm, symtab);
                resolve_expression(expr->lterm, symtab);
                break;
            case BIN_LT:
                resolve_expression(expr->lterm, symtab);
                resolve_expression(expr->rterm, symtab);
                break;
            case BIN_LTE:
                resolve_expression(expr->lterm, symtab);
                resolve_expression(expr->rterm, symtab);
                break;
            case BIN_GT:
                resolve_expression(expr->lterm, symtab);
                resolve_expression(expr->rterm, symtab);
                break;
            case BIN_GTE:
                resolve_expression(expr->lterm, symtab);
                resolve_expression(expr->rterm, symtab);
                break;
            case BIN_AND:
                resolve_expression(expr->lterm, symtab);
                resolve_expression(expr->rterm, symtab);
                break;
            case BIN_OR:
                resolve_expression(expr->lterm, symtab);
                resolve_expression(expr->rterm, symtab);
                break;
            default:
                printf("Error: Unknown binary operator.\n");
                exit(1);
        }
    }

    else if (expr->type == EXPR_ASS) {
        int offset = symtab_lookup(symtab, expr->lterm->text);
        if (offset == -1) {
            printf("Error: Variable '%s' is not defined in this scope\n", expr->lterm->text);
            exit(1);
        }
        expr->lterm->resolved_offset = offset;
        resolve_expression(expr->rterm, symtab);
    }

    else if (expr->type == EXPR_VAR) {
        int offset = symtab_lookup(symtab, expr->text);
        if (offset == -1) {
            printf("Error: Variable '%s' is not defined in this scope\n", expr->text);
            exit(1);
        }
        expr->resolved_offset = offset;
    }

    else if (expr->type == EXPR_TERNARY) {
        resolve_expression(expr->term_cond, symtab);
        resolve_expression(expr->term_one, symtab);
        resolve_expression(expr->term_two, symtab);
    }

    return;
}

// symtab pass by value instead of pass by reference so local variables created in an inner scope do not exist in the outer scope
static void resolve_statement(Statement *stmt, SymbolTable symtab) {
    if (stmt->type == STMT_RETURN) {
        resolve_expression(stmt->expr, &symtab);
        return;
    }

    if (stmt->type == STMT_BLOCK) {
        symtab.scope_count = symtab.sym_count; // new scope for local variables
        BlockItem *curr = stmt->block_head;
        while (curr != NULL) {
            resolve_block_item(curr, &symtab);
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
        resolve_expression(stmt->expr, &symtab);
    }

    else if (stmt->type == STMT_COND) {
        resolve_expression(stmt->expr, &symtab);
        resolve_statement(stmt->if_stmt, symtab);

        if (stmt->else_stmt != NULL) {
            resolve_statement(stmt->else_stmt, symtab);
        }   
    }

    else if (stmt->type == STMT_WHILE) {
        resolve_expression(stmt->expr, &symtab);
        resolve_statement(stmt->loop_stmt, symtab);
    }

    else if (stmt->type == STMT_DO_WHILE) {
        resolve_statement(stmt->loop_stmt, symtab);
        resolve_expression(stmt->expr, &symtab);
        return;
    }

    else if (stmt->type == STMT_CONT) {
        //TODO
        return;
    }

    else if (stmt->type == STMT_BREAK) {
        //TODO
        return;
    }

    return;
}

static void resolve_declaration(Declaration *decl, SymbolTable *symtab) {
    if (decl->type == DECL_INT) {
        if (symtab_lookup_in_scope(symtab, decl->name) != -1) {
            printf("Error: variable with name %s is already defined\n", decl->name);
            exit(1);
        }

        if (decl->expr != NULL) {
            resolve_expression(decl->expr, symtab);
        }
        int offset = symtab_add(symtab, decl->name);
        decl->resolved_offset = offset;
    }
    update_largest_offset(symtab->curr_offset);
    return;
}

static void resolve_block_item(BlockItem *bi, SymbolTable *symtab) {
    if (bi->type == BLCKITEM_STMT) {
        resolve_statement(bi->stmt, *symtab);
    }

    else if (bi->type == BLCKITEM_DECL) {
        resolve_declaration(bi->decl, symtab);
    }

    return;
}

static void resolve_function(Function *fnctn) {
    largest_offset = 0;
    SymbolTable symtab;
    symtab_initialize(&symtab);

    resolve_statement(fnctn->stmt, symtab);

    // round to next highest multiple of 16
    fnctn->frame_size = (largest_offset + 15) & ~15;
}

void resolve_code(Program *prog) {
    resolve_function(prog->fnctn);
}
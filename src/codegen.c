#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "codegen.h"
#include "symtab.h"

static void codegen_expression(Expression *expr, SymbolTable *symtab, FILE *out);
static void codegen_statement(Statement *stmt, SymbolTable symtab, FILE *out);
static void codegen_declaration(Declaration *decl, SymbolTable *symtab, FILE *out);
static void codegen_block_item(BlockItem *bi, SymbolTable *symtab, FILE *out);
static void codegen_function(Function *fnctn, FILE *out);

int clause_count = 0;

int get_clause_count() {
    return clause_count++;
}

static void codegen_expression(Expression *expr, SymbolTable *symtab, FILE *out) {
    if (expr->type == EXPR_CONST) {
        fprintf(out, "    movq $%ld, %%rax\n", (long) expr->int_val);
    }
    
    else if (expr->type == EXPR_UNOP) {
        switch (expr->un_op) {
            case OP_NEG:
                codegen_expression(expr->lterm, symtab, out);
                fprintf(out, "    neg %%rax\n");
                break;
            case OP_COMPL:
                codegen_expression(expr->lterm, symtab, out);
                fprintf(out, "    not %%rax\n");
                break;
            case OP_NOT:
                codegen_expression(expr->lterm, symtab, out);
                fprintf(out, "    cmpq $0, %%rax\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    sete %%al\n");
                break;
            default:
                printf("Error: Could not generate code for unary operator.\n");
                exit(1);
        }
    }

    else if (expr->type == EXPR_BINOP) {
        int clause_count;
        switch (expr->bin_op) {
            case BIN_NEG:
                codegen_expression(expr->rterm, symtab, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->lterm, symtab, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    subq %%rcx, %%rax\n");
                break;
            case BIN_ADD:
                codegen_expression(expr->rterm, symtab, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->lterm, symtab, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    addq %%rcx, %%rax\n");
                break;
            case BIN_MULTIPLY:
                codegen_expression(expr->rterm, symtab, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->lterm, symtab, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    imulq %%rcx, %%rax\n");
                break;
            case BIN_DIVIDE:
                codegen_expression(expr->rterm, symtab, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->lterm, symtab, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    cqo\n");
                fprintf(out, "    idivq %%rcx\n");
                break;
            case BIN_MOD:
                codegen_expression(expr->rterm, symtab, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->lterm, symtab, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    cqo\n");
                fprintf(out, "    idivq %%rcx\n");
                fprintf(out, "    movq %%rdx, %%rax\n");
                break;
            case BIN_EQ:
                codegen_expression(expr->rterm, symtab, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->lterm, symtab, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    cmpq %%rax, %%rcx\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    sete %%al\n");
                break;
            case BIN_NEQ:
                codegen_expression(expr->rterm, symtab, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->lterm, symtab, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    cmpq %%rax, %%rcx\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    setne %%al\n");
                break;
            case BIN_LT:
                codegen_expression(expr->lterm, symtab, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->rterm, symtab, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    cmpq %%rax, %%rcx\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    setl %%al\n");
                break;
            case BIN_LTE:
                codegen_expression(expr->lterm, symtab, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->rterm, symtab, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    cmpq %%rax, %%rcx\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    setle %%al\n");
                break;
            case BIN_GT:
                codegen_expression(expr->lterm, symtab, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->rterm, symtab, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    cmpq %%rax, %%rcx\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    setg %%al\n");
                break;
            case BIN_GTE:
                codegen_expression(expr->lterm, symtab, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->rterm, symtab, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    cmpq %%rax, %%rcx\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    setge %%al\n");
                break;
            case BIN_AND:
                clause_count = get_clause_count();
                codegen_expression(expr->lterm, symtab, out);
                fprintf(out, "    cmpq $0, %%rax\n");
                fprintf(out, "    jne _clause%d\n", clause_count);
                fprintf(out, "    jmp _end%d\n", clause_count);
                fprintf(out, "_clause%d:\n", clause_count);
                codegen_expression(expr->rterm, symtab, out);
                fprintf(out, "    cmpq $0, %%rax\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    setne %%al\n");
                fprintf(out, "_end%d:\n", clause_count);
                break;
            case BIN_OR:
                clause_count = get_clause_count();
                codegen_expression(expr->lterm, symtab, out);
                fprintf(out, "    cmpq $0, %%rax\n");
                fprintf(out, "    je _clause%d\n", clause_count);
                fprintf(out, "    movq $1, %%rax\n");
                fprintf(out, "    jmp _end%d\n", clause_count);
                fprintf(out, "_clause%d:\n", clause_count);
                codegen_expression(expr->rterm, symtab, out);
                fprintf(out, "    cmpq $0, %%rax\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    setne %%al\n");
                fprintf(out, "_end%d:\n", clause_count);
                break;
            default:
                printf("Error: Could not generate code for binary operator.\n");
                exit(1);
        }
    }

    else if (expr->type == EXPR_ASS) {
        int offset = symtab_lookup(symtab, expr->lterm->text);
        if (offset == -1) {
            printf("Error: Variable '%s' is not defined in this scope\n", expr->lterm->text);
            exit(1);
        }
        codegen_expression(expr->rterm, symtab, out);
        fprintf(out, "    movl %%eax, %d(%%rbp)\n", offset);
    }

    else if (expr->type == EXPR_VAR) {
        int offset = symtab_lookup(symtab, expr->text);
        if (offset == -1) {
            printf("Error: Variable '%s' is not defined in this scope\n", expr->text);
            exit(1);
        }
        fprintf(out, "    movl %d(%%rbp), %%eax\n", offset);
    }

    else if (expr->type == EXPR_TERNARY) {
        int clause_count = get_clause_count();

        codegen_expression(expr->term_cond, symtab, out);
        fprintf(out, "    cmpq $0, %%rax\n");
        fprintf(out, "    je _ternary_two%d\n", clause_count);

        codegen_expression(expr->term_one, symtab, out);
        fprintf(out, "    jmp _ternary_end%d\n", clause_count);

        fprintf(out, "_ternary_two%d:\n", clause_count);
        codegen_expression(expr->term_two, symtab, out);
        fprintf(out, "_ternary_end%d:\n", clause_count);
    }
}

// symtab pass by value instead of pass by reference so local variables created in an inner scope do not exist in the outer scope
static void codegen_statement(Statement *stmt, SymbolTable symtab, FILE *out) {
    if (stmt->type == STMT_RETURN) {
        codegen_expression(stmt->expr, &symtab, out);
        // epilogue
        fprintf(out, "    movq %%rbp, %%rsp\n");
        fprintf(out, "    popq %%rbp\n");
        fprintf(out, "    ret\n");
        return;
    }

    if (stmt->type == STMT_BLOCK) {
        symtab.scope_count = symtab.sym_count; // new scope for local variables
        BlockItem *curr = stmt->block_head;
        while (curr != NULL) {
            codegen_block_item(curr, &symtab, out);
            
            if (curr->type == BLCKITEM_STMT && curr->stmt->type == STMT_RETURN) {
                if (curr->next != NULL) {
                    printf("Warning: Code will never be reached\n");
                }
                break;
            }

            curr = curr->next;
        }
        int num_bytes_deallocate = 4 * (symtab.sym_count - symtab.scope_count);
        fprintf(out, "    addq $%d, %%rsp\n", num_bytes_deallocate);
    }

    else if (stmt->type == STMT_EXPR) {
        codegen_expression(stmt->expr, &symtab, out);
    }

    else if (stmt->type == STMT_COND) {
        int clause_count = get_clause_count();
        codegen_expression(stmt->expr, &symtab, out);
        fprintf(out, "    cmpq $0, %%rax\n");
        if (stmt->else_stmt == NULL) {
            fprintf(out, "    je _condition_end%d\n", clause_count);
        } else {
            fprintf(out, "    je _condition_else%d\n", clause_count);
        }
        codegen_statement(stmt->if_stmt, symtab, out);

        if (stmt->else_stmt != NULL) {
            fprintf(out, "    jmp _condition_end%d\n", clause_count);
            fprintf(out, "_condition_else%d:\n", clause_count);
            codegen_statement(stmt->else_stmt, symtab, out);
        }
        
        fprintf(out, "_condition_end%d:\n", clause_count);
    }

    return;
}

static void codegen_declaration(Declaration *decl, SymbolTable *symtab, FILE *out) {
    if (decl->type == DECL_INT) {
        if (symtab_lookup_in_scope(symtab, decl->name) != -1) {
            printf("Error: variable with name %s is already defined\n", decl->name);
            exit(1);
        }

        if (decl->expr != NULL) {
            codegen_expression(decl->expr, symtab, out);
        } else {
            fprintf(out, "    movl $0, %%eax\n");
        }
        
        // fprintf(out, "    pushq %%rax\n"); cant use pushq as it stores 64 bits instead of 32

        int offset = symtab_add(symtab, decl->name);

        fprintf(out, "    subq $4, %%rsp\n");
        fprintf(out, "    movl %%eax, %d(%%rbp)\n", offset);
    }

    return;
}

static void codegen_block_item(BlockItem *bi, SymbolTable *symtab, FILE *out) {
    if (bi->type == BLCKITEM_STMT) {
        codegen_statement(bi->stmt, *symtab, out);
    }

    else if (bi->type == BLCKITEM_DECL) {
        codegen_declaration(bi->decl, symtab, out);
    }

    return;
}

static void codegen_function(Function *fnctn, FILE *out) {
    fprintf(out, "    .globl %s\n", fnctn->name);
    fprintf(out, "%s:\n", fnctn->name);

    // prologue
    fprintf(out, "    pushq %%rbp\n");
    fprintf(out, "    movq %%rsp, %%rbp\n");

    SymbolTable symtab;
    symtab_initialize(&symtab);

    codegen_statement(fnctn->stmt, symtab, out);

    if (strcmp(fnctn->name, "main") == 0) {
        // safety incase no return was specified
        fprintf(out, "    movq %%rbp, %%rsp\n");
        fprintf(out, "    popq %%rbp\n");
        fprintf(out, "    movl $0, %%eax\n");
        fprintf(out, "    ret\n");
    }
}

void generate_code(Program *prog, FILE *out) {
    codegen_function(prog->fnctn, out);
}
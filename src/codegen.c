#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "codegen.h"

int clause_count = 0;

int get_clause_count() {
    return clause_count++;
}

static void codegen_expression(Expression *expr, FILE *out) {
    if (expr->type == EXPR_CONST) {
        fprintf(out, "    movq $%ld, %%rax\n", (long) expr->int_val);
    }
    
    else if (expr->type == EXPR_UNOP) {
        switch (expr->un_op) {
            case OP_NEG:
                codegen_expression(expr->lterm, out);
                fprintf(out, "    neg %%rax\n");
                break;
            case OP_COMPL:
                codegen_expression(expr->lterm, out);
                fprintf(out, "    not %%rax\n");
                break;
            case OP_NOT:
                codegen_expression(expr->lterm, out);
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
                codegen_expression(expr->rterm, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->lterm, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    subq %%rcx, %%rax\n");
                break;
            case BIN_ADD:
                codegen_expression(expr->rterm, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->lterm, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    addq %%rcx, %%rax\n");
                break;
            case BIN_MULTIPLY:
                codegen_expression(expr->rterm, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->lterm, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    imulq %%rcx, %%rax\n");
                break;
            case BIN_DIVIDE:
                codegen_expression(expr->rterm, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->lterm, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    cqo\n");
                fprintf(out, "    idivq %%rcx\n");
                break;
            case BIN_MOD:
                codegen_expression(expr->rterm, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->lterm, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    cqo\n");
                fprintf(out, "    idivq %%rcx\n");
                fprintf(out, "    movq %%rdx, %%rax\n");
                break;
            case BIN_EQ:
                codegen_expression(expr->rterm, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->lterm, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    cmpq %%rax, %%rcx\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    sete %%al\n");
                break;
            case BIN_NEQ:
                codegen_expression(expr->rterm, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->lterm, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    cmpq %%rax, %%rcx\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    setne %%al\n");
                break;
            case BIN_LT:
                codegen_expression(expr->lterm, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->rterm, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    cmpq %%rax, %%rcx\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    setl %%al\n");
                break;
            case BIN_LTE:
                codegen_expression(expr->lterm, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->rterm, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    cmpq %%rax, %%rcx\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    setle %%al\n");
                break;
            case BIN_GT:
                codegen_expression(expr->lterm, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->rterm, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    cmpq %%rax, %%rcx\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    setg %%al\n");
                break;
            case BIN_GTE:
                codegen_expression(expr->lterm, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->rterm, out);
                fprintf(out, "    pop %%rcx\n");
                fprintf(out, "    cmpq %%rax, %%rcx\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    setge %%al\n");
                break;
            case BIN_AND:
                clause_count = get_clause_count();
                codegen_expression(expr->lterm, out);
                fprintf(out, "    cmpq $0, %%rax\n");
                fprintf(out, "    jne _clause%d\n", clause_count);
                fprintf(out, "    jmp _end%d\n", clause_count);
                fprintf(out, "_clause%d:\n", clause_count);
                codegen_expression(expr->rterm, out);
                fprintf(out, "    cmpq $0, %%rax\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    setne %%al\n");
                fprintf(out, "_end%d:\n", clause_count);
                break;
            case BIN_OR:
                clause_count = get_clause_count();
                codegen_expression(expr->lterm, out);
                fprintf(out, "    cmpq $0, %%rax\n");
                fprintf(out, "    je _clause%d\n", clause_count);
                fprintf(out, "    movq $1, %%rax\n");
                fprintf(out, "    jmp _end%d\n", clause_count);
                fprintf(out, "_clause%d:\n", clause_count);
                codegen_expression(expr->rterm, out);
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
}

static void codegen_statement(Statement *stmt, FILE *out) {
    if (stmt->type == STMT_RETURN) {
        codegen_expression(stmt->expr, out);
        fprintf(out, "    ret\n");
    }
}

static void codegen_function(Function *fnctn, FILE *out) {
    fprintf(out, "    .globl %s\n", fnctn->name);
    fprintf(out, "%s:\n", fnctn->name);
    codegen_statement(fnctn->stmt, out);
}

void generate_code(Program *prog, FILE *out) {
    codegen_function(prog->fnctn, out);
}
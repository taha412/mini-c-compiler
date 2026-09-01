#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "codegen.h"

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
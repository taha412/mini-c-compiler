#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "codegen.h"

static void codegen_expression(Expression *expr, FILE *out) {
    if (expr->type == EXPR_INT_LIT) {
        fprintf(out, "    movl $%ld, %%eax\n", (long) expr->val);
    } else if (expr->type == EXPR_UNARY) {
        switch (expr->op) {
            case OP_NEG:
                codegen_expression(expr->operand, out);
                fprintf(out, "    neg %%eax\n");
                break;
            case OP_COMPL:
                codegen_expression(expr->operand, out);
                fprintf(out, "    not %%eax\n");
                break;
            case OP_NOT:
                codegen_expression(expr->operand, out);
                fprintf(out, "    cmpl $0, %%eax\n");
                fprintf(out, "    movl $0, %%eax\n");
                fprintf(out, "    sete %%al\n");
                break;
            default:
                printf("Error: Could not generate code for unary operator.\n");
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
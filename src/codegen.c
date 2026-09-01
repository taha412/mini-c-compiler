#include <stdio.h>

#include "parser.h"
#include "codegen.h"

static void codegen_expression(Expression *expr, FILE *out) {
    if (expr->type == EXPR_INT_LIT) {
        fprintf(out, "    movl $%ld, %%eax\n", (long) expr->val);
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
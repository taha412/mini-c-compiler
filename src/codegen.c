#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "codegen.h"

static void codegen_expression(Expression *expr, FILE *out);
static void codegen_statement(Statement *stmt, FILE *out);
static void codegen_declaration(Declaration *decl, FILE *out);
static void codegen_block_item(BlockItem *bi, FILE *out);
static void codegen_function(Function *fnctn, FILE *out);

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
                codegen_expression(expr->lterm, out);
                fprintf(out, "    push %%rax\n");
                codegen_expression(expr->rterm, out);
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
                codegen_expression(expr->lterm, out);
                fprintf(out, "    cmpq $0, %%rax\n");
                fprintf(out, "    jne _clause%d\n", expr->clause_count);
                fprintf(out, "    jmp _end%d\n", expr->clause_count);
                fprintf(out, "_clause%d:\n", expr->clause_count);
                codegen_expression(expr->rterm, out);
                fprintf(out, "    cmpq $0, %%rax\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    setne %%al\n");
                fprintf(out, "_end%d:\n", expr->clause_count);
                break;
            case BIN_OR:
                codegen_expression(expr->lterm, out);
                fprintf(out, "    cmpq $0, %%rax\n");
                fprintf(out, "    je _clause%d\n", expr->clause_count);
                fprintf(out, "    movq $1, %%rax\n");
                fprintf(out, "    jmp _end%d\n", expr->clause_count);
                fprintf(out, "_clause%d:\n", expr->clause_count);
                codegen_expression(expr->rterm, out);
                fprintf(out, "    cmpq $0, %%rax\n");
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    setne %%al\n");
                fprintf(out, "_end%d:\n", expr->clause_count);
                break;
            default:
                printf("Error: Could not generate code for binary operator.\n");
                exit(1);
        }
    }

    else if (expr->type == EXPR_ASS) {
        // validated during resolve
        codegen_expression(expr->rterm, out);
        fprintf(out, "    movl %%eax, %d(%%rbp)\n", expr->lterm->resolved_offset);
    }

    else if (expr->type == EXPR_VAR) {
        fprintf(out, "    movl %d(%%rbp), %%eax\n", expr->resolved_offset);
    }

    else if (expr->type == EXPR_TERNARY) {
        codegen_expression(expr->term_cond, out);
        fprintf(out, "    cmpq $0, %%rax\n");
        fprintf(out, "    je _ternary_two%d\n", expr->clause_count);

        codegen_expression(expr->term_one, out);
        fprintf(out, "    jmp _ternary_end%d\n", expr->clause_count);

        fprintf(out, "_ternary_two%d:\n", expr->clause_count);
        codegen_expression(expr->term_two, out);
        fprintf(out, "_ternary_end%d:\n", expr->clause_count);
    }
}

static void codegen_statement(Statement *stmt, FILE *out) {
    if (stmt->type == STMT_RETURN) {
        codegen_expression(stmt->expr, out);
        // epilogue
        fprintf(out, "    movq %%rbp, %%rsp\n");
        fprintf(out, "    popq %%rbp\n");
        fprintf(out, "    ret\n");
        return;
    }

    if (stmt->type == STMT_BLOCK) {
        BlockItem *curr = stmt->block_head;
        while (curr != NULL) {
            codegen_block_item(curr, out);
            curr = curr->next;
        }
    }

    else if (stmt->type == STMT_EXPR) {
        codegen_expression(stmt->expr, out);
    }

    else if (stmt->type == STMT_COND) {
        codegen_expression(stmt->expr, out);
        fprintf(out, "    cmpq $0, %%rax\n");
        if (stmt->else_stmt == NULL) {
            fprintf(out, "    je _condition_end%d\n", stmt->clause_count);
        } else {
            fprintf(out, "    je _condition_else%d\n", stmt->clause_count);
        }
        codegen_statement(stmt->if_stmt, out);

        if (stmt->else_stmt != NULL) {
            fprintf(out, "    jmp _condition_end%d\n", stmt->clause_count);
            fprintf(out, "_condition_else%d:\n", stmt->clause_count);
            codegen_statement(stmt->else_stmt, out);
        }
        
        fprintf(out, "_condition_end%d:\n", stmt->clause_count);
    }

    else if (stmt->type == STMT_FOR) {
        if (stmt->init != NULL) {
            codegen_block_item(stmt->init, out);
        }

        fprintf(out, "_for_loop%d:\n", stmt->clause_count);

        if (stmt->cond != NULL) {
            codegen_expression(stmt->cond, out);
            fprintf(out, "    cmpq $0, %%rax\n");
            fprintf(out, "    je _loop_end%d\n", stmt->clause_count);
        }

        codegen_statement(stmt->loop_stmt, out);

        if (stmt->post != NULL) {
            codegen_expression(stmt->post, out);
        }
        fprintf(out, "    jmp _for_loop%d\n", stmt->clause_count);
        fprintf(out, "_loop_end%d:\n", stmt->clause_count);
        return;
        
    }

    else if (stmt->type == STMT_WHILE) {
        fprintf(out, "_while%d:\n", stmt->clause_count);

        codegen_expression(stmt->expr, out);
        fprintf(out, "    cmpq $0, %%rax\n");
        fprintf(out, "    je _loop_end%d\n", stmt->clause_count);

        codegen_statement(stmt->loop_stmt, out);

        fprintf(out, "    jmp _while%d\n", stmt->clause_count);
        fprintf(out, "_loop_end%d:\n", stmt->clause_count);
    }

    else if (stmt->type == STMT_DO_WHILE) {
        fprintf(out, "_do_while%d:\n", stmt->clause_count);

        codegen_statement(stmt->loop_stmt, out);

        codegen_expression(stmt->expr, out);
        fprintf(out, "    cmpq $0, %%rax\n");
        fprintf(out, "    jne _do_while%d\n", stmt->clause_count);
        fprintf(out, "_loop_end%d:\n", stmt->clause_count);
        return;
    }

    else if (stmt->type == STMT_CONT) {
        // TODO
        return;
    }

    else if (stmt->type == STMT_BREAK) {
        // TODO
        return;
    }

    return;
}

static void codegen_declaration(Declaration *decl, FILE *out) {
    if (decl->type == DECL_INT) {
        if (decl->expr != NULL) {
            codegen_expression(decl->expr, out);
        } else {
            fprintf(out, "    movl $0, %%eax\n");
        }
        
        fprintf(out, "    movl %%eax, %d(%%rbp)\n", decl->resolved_offset);
    }

    return;
}

static void codegen_block_item(BlockItem *bi, FILE *out) {
    if (bi->type == BLCKITEM_STMT) {
        codegen_statement(bi->stmt, out);
    }

    else if (bi->type == BLCKITEM_DECL) {
        codegen_declaration(bi->decl, out);
    }

    return;
}

static void codegen_function(Function *fnctn, FILE *out) {
    fprintf(out, "    .globl %s\n", fnctn->name);
    fprintf(out, "%s:\n", fnctn->name);

    // prologue
    fprintf(out, "    pushq %%rbp\n");
    fprintf(out, "    movq %%rsp, %%rbp\n");
    
    if (fnctn->frame_size > 0) {
        fprintf(out, "    subq $%d, %%rsp\n", fnctn->frame_size);
    }

    codegen_statement(fnctn->stmt, out);

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
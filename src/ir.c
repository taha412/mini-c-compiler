#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ir.h"
#include "parser.h"





static void print_operand(Operand *op) {
    if (op == NULL) {
        return;
    }

    switch (op->type) {
        case OPERAND_CONST:
            printf("%d", op->value.constant);
            break;

        case OPERAND_VAR:
            printf("v[%d]", op->value.offset);
            break;

        case OPERAND_TEMP:
            printf("t%d", op->value.temp_id);
            break;
    }
}

static void print_args(IRArg *arg) {
    while (arg != NULL) {
        print_operand(arg->operand);

        if (arg->next != NULL) {
            printf(", ");
        }

        arg = arg->next;
    }
}

static void print_instruction(IRInstruction *iri) {
    switch (iri->operation) {
        case IR_COPY:
            print_operand(iri->dest);
            printf(" = ");
            print_operand(iri->src1);
            break;

        case IR_ADD:
            print_operand(iri->dest);
            printf(" = ");
            print_operand(iri->src1);
            printf(" + ");
            print_operand(iri->src2);
            break;

        case IR_SUB:
            print_operand(iri->dest);
            printf(" = ");
            print_operand(iri->src1);
            printf(" - ");
            print_operand(iri->src2);
            break;

        case IR_MUL:
            print_operand(iri->dest);
            printf(" = ");
            print_operand(iri->src1);
            printf(" * ");
            print_operand(iri->src2);
            break;

        case IR_DIV:
            print_operand(iri->dest);
            printf(" = ");
            print_operand(iri->src1);
            printf(" / ");
            print_operand(iri->src2);
            break;

        case IR_MOD:
            print_operand(iri->dest);
            printf(" = ");
            print_operand(iri->src1);
            printf(" %% ");
            print_operand(iri->src2);
            break;

        case IR_EQ:
            print_operand(iri->dest);
            printf(" = ");
            print_operand(iri->src1);
            printf(" == ");
            print_operand(iri->src2);
            break;

        case IR_NEQ:
            print_operand(iri->dest);
            printf(" = ");
            print_operand(iri->src1);
            printf(" != ");
            print_operand(iri->src2);
            break;

        case IR_LT:
            print_operand(iri->dest);
            printf(" = ");
            print_operand(iri->src1);
            printf(" < ");
            print_operand(iri->src2);
            break;

        case IR_LTE:
            print_operand(iri->dest);
            printf(" = ");
            print_operand(iri->src1);
            printf(" <= ");
            print_operand(iri->src2);
            break;

        case IR_GT:
            print_operand(iri->dest);
            printf(" = ");
            print_operand(iri->src1);
            printf(" > ");
            print_operand(iri->src2);
            break;

        case IR_GTE:
            print_operand(iri->dest);
            printf(" = ");
            print_operand(iri->src1);
            printf(" >= ");
            print_operand(iri->src2);
            break;

        case IR_NEG:
            print_operand(iri->dest);
            printf(" = -");
            print_operand(iri->src1);
            break;

        case IR_COMPL:
            print_operand(iri->dest);
            printf(" = ~");
            print_operand(iri->src1);
            break;

        case IR_NOT:
            print_operand(iri->dest);
            printf(" = !");
            print_operand(iri->src1);
            break;

        case IR_RETURN:
            printf("RETURN ");
            print_operand(iri->src1);
            break;

        case IR_CALL:
            if (iri->dest != NULL) {
                print_operand(iri->dest);
                printf(" = ");
            }

            printf("CALL %s(", iri->name);
            print_args(iri->args);
            printf(")");
            break;

        case IR_AND:
            print_operand(iri->dest);
            printf(" = ");
            print_operand(iri->src1);
            printf(" && ");
            print_operand(iri->src2);
            break;

        case IR_OR:
            print_operand(iri->dest);
            printf(" = ");
            print_operand(iri->src1);
            printf(" || ");
            print_operand(iri->src2);
            break;
    }

    printf("\n");
}

static void print_ir_function(IRFunction *irf) {
    printf("--- IR ---\n");

    for (IRInstruction *iri = irf->head;
         iri != NULL;
         iri = iri->next) {
        print_instruction(iri);
    }

    printf("----------\n");
}











static void lower_block_item(BlockItem *bi, IRFunction *irf);

int temp_count = 0;

static Operand *make_const(int con) {
    Operand *op = (Operand *) calloc(1, sizeof(Operand));
    op->type = OPERAND_CONST;
    op->value.constant = con;
    return op;
}

static Operand *make_var(int offset) {
    Operand *op = (Operand *) calloc(1, sizeof(Operand));
    op->type = OPERAND_VAR;
    op->value.offset = offset;
    return op;
}

static Operand *make_temp() {
    Operand *op = (Operand *) calloc(1, sizeof(Operand));
    op->type = OPERAND_TEMP;
    op->value.temp_id = temp_count;
    temp_count++;
    return op;
}

static IRInstruction *emit_instruction(IRFunction *irf, IROp operation, Operand *dest, Operand *src1, Operand *src2) {
    IRInstruction *iri = (IRInstruction *) calloc(1, sizeof(IRInstruction));
    iri->operation = operation;
    iri->dest = dest;
    iri->src1 = src1;
    iri->src2 = src2;
    iri->next = NULL;
    if (irf->head == NULL) {
        irf->head = iri;
        irf->tail = iri;
    } else {
        irf->tail->next = iri;
        irf->tail = iri;
    }
    return iri;
}

static Operand *lower_expression(Expression *expr, IRFunction *irf) {    
    if (expr->type == EXPR_UNOP) {
        Operand *s;
        Operand *d;
        switch (expr->un_op) {
            case OP_NEG:
                s = lower_expression(expr->lterm, irf);
                d = make_temp();
                emit_instruction(irf, IR_NEG, d, s, NULL);
                return d;
            case OP_COMPL:
                s = lower_expression(expr->lterm, irf);
                d = make_temp();
                emit_instruction(irf, IR_COMPL, d, s, NULL);
                return d;
            case OP_NOT:
                s = lower_expression(expr->lterm, irf);
                d = make_temp();
                emit_instruction(irf, IR_NOT, d, s, NULL);
                return d;
            default:
                printf("Error: Unknown unary operator.\n");
                exit(1);
        }
    }

    else if (expr->type == EXPR_BINOP) {
        Operand *s1 = lower_expression(expr->lterm, irf);
        Operand *s2 = lower_expression(expr->rterm, irf);
        Operand *d = make_temp();

        switch (expr->bin_op) {
            case BIN_NEG:
                emit_instruction(irf, IR_SUB, d, s1, s2);
                return d;
            case BIN_ADD:
                emit_instruction(irf, IR_ADD, d, s1, s2);
                return d;
            case BIN_MULTIPLY:
                emit_instruction(irf, IR_MUL, d, s1, s2);
                return d;
            case BIN_DIVIDE:
                emit_instruction(irf, IR_DIV, d, s1, s2);
                return d;
            case BIN_MOD:
                emit_instruction(irf, IR_MOD, d, s1, s2);
                return d;
            case BIN_EQ:
                emit_instruction(irf, IR_EQ, d, s1, s2);
                return d;
            case BIN_NEQ:
                emit_instruction(irf, IR_NEQ, d, s1, s2);
                return d;
            case BIN_LT:
                emit_instruction(irf, IR_LT, d, s1, s2);
                return d;
            case BIN_LTE:
                emit_instruction(irf, IR_LTE, d, s1, s2);
                return d;
            case BIN_GT:
                emit_instruction(irf, IR_GT, d, s1, s2);
                return d;
            case BIN_GTE:
                emit_instruction(irf, IR_GTE, d, s1, s2);
                return d;
            case BIN_AND:
                emit_instruction(irf, IR_AND, d, s1, s2);
                return d;
            case BIN_OR:
                emit_instruction(irf, IR_OR, d, s1, s2);
                return d;
            default:
                printf("Error: Unknown binary operator.\n");
                exit(1);
        }
    }

    else if (expr->type == EXPR_CONST) {
        return make_const(expr->int_val);
    }

    else if (expr->type == EXPR_ASS) {
        Operand *s = lower_expression(expr->rterm, irf);
        Operand *d = make_var(expr->lterm->resolved_offset);
        emit_instruction(irf, IR_COPY, d, s, NULL);
        return d;
    }

    else if (expr->type == EXPR_VAR) {
        return make_var(expr->resolved_offset);
    }

    else if (expr->type == EXPR_TERNARY) {
        lower_expression(expr->term_cond, irf);
        lower_expression(expr->term_one, irf);
        lower_expression(expr->term_two, irf);
    }

    else if (expr->type == EXPR_CALL) {
        //TODO Add type checking later
        IRArg *curr = NULL;
        IRArg *head = NULL;
        for (Argument *arg = expr->args; arg != NULL; arg = arg->next) {
            IRArg *ia = (IRArg *) calloc(1, sizeof(IRArg));
            ia->operand = lower_expression(arg->expr, irf);
            if (head == NULL) {
                curr = ia;
                head = ia;
            } else {
                curr->next = ia;
                curr = ia;
            }
        }
        Operand *d = make_temp();
        IRInstruction *iri = emit_instruction(irf, IR_CALL, d, NULL, NULL);
        iri->args = head;
        iri->arg_count = expr->arg_count;
        strncpy(iri->name, expr->text, 63);
        iri->name[63] = '\0';
        return d;
    }

    return NULL;
}

// symtab pass by value instead of pass by reference so local variables created in an inner scope do not exist in the outer scope
static void lower_statement(Statement *stmt, IRFunction *irf) {
    if (stmt->type == STMT_RETURN) {
        Operand *ret = lower_expression(stmt->expr, irf);
        emit_instruction(irf, IR_RETURN, NULL, ret, NULL);
        return;
    }

    if (stmt->type == STMT_BLOCK) {
        BlockItem *curr = stmt->block_head;
        while (curr != NULL) {
            lower_block_item(curr, irf);
            if (curr->type == BLCKITEM_STMT && curr->stmt->type == STMT_RETURN) {
                break;
            }
            curr = curr->next;
        }
    }

    else if (stmt->type == STMT_EXPR) {
        lower_expression(stmt->expr, irf);
    }

    else if (stmt->type == STMT_COND) {
        lower_expression(stmt->expr, irf);
        lower_statement(stmt->if_stmt, irf);

        if (stmt->else_stmt != NULL) {
            lower_statement(stmt->else_stmt, irf);
        }   
    }

    else if (stmt->type == STMT_FOR) {
        if (stmt->init != NULL) {
            lower_block_item(stmt->init, irf);
        }

        if (stmt->cond != NULL) {
            lower_expression(stmt->cond, irf);
        }

        lower_statement(stmt->loop_stmt, irf);

        if (stmt->post != NULL) {
            lower_expression(stmt->post, irf);
        }
    }

    else if (stmt->type == STMT_WHILE) {
        lower_expression(stmt->expr, irf);
        lower_statement(stmt->loop_stmt, irf);
    }

    else if (stmt->type == STMT_DO_WHILE) {
        lower_statement(stmt->loop_stmt, irf);
        lower_expression(stmt->expr, irf);
        return;
    }

    else if (stmt->type == STMT_CONT) {
        return;
    }

    else if (stmt->type == STMT_BREAK) {
        return;
    }

    else if (stmt->type == STMT_NULL) {
        return;
    }

    return;
}

static void lower_declaration(Declaration *decl, IRFunction *irf) {
    if (decl->data_type == DATA_INT) {
        if (decl->expr != NULL) {
            Operand *s = lower_expression(decl->expr, irf);
            Operand *d = make_var(decl->resolved_offset);
            emit_instruction(irf, IR_COPY, d, s, NULL);
            return;
        }
    }
    return;
}

static void lower_block_item(BlockItem *bi, IRFunction *irf) {
    if (bi->type == BLCKITEM_STMT) {
        lower_statement(bi->stmt, irf);
    }

    else if (bi->type == BLCKITEM_DECL) {
        lower_declaration(bi->decl, irf);
    }

    return;
}

static IRFunction *lower_function(Function *fnctn) {
    temp_count = 0;
    IRFunction *irf = (IRFunction *) calloc(1, sizeof(IRFunction));

    strncpy(irf->name, fnctn->name, 63);
    irf->name[63] = '\0'; // safety

    lower_statement(fnctn->stmt, irf);

    print_ir_function(irf);

    return irf;
}

static void lower_top_lvl_item(TopLevelItem *tpi, IRProgram *irp) {
    if (tpi->type == TOPLVL_FNCTN) {
        if (tpi->fnctn->stmt != NULL) { // TODO: Maybe move null check to lower_function
            IRFunction *irf = lower_function(tpi->fnctn);
            if (irp->head == NULL) {
                irp->head = irf;
                irp->tail = irf;
            } else {
                irp->tail->next = irf;
                irp->tail = irf;

            }
        }
    }
}

IRProgram *lower_code(Program *prog) {
    TopLevelItem *curr = prog->top;
    IRProgram *irp = calloc(1, sizeof(IRProgram));

    while (curr != NULL) {
        if (curr->type == TOPLVL_FNCTN) {
            if (curr->fnctn->stmt != NULL) {
                lower_top_lvl_item(curr, irp);
            }
        }
        curr = curr->next;
    }

    return irp;
}
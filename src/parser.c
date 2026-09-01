#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lexer.h"
#include "parser.h"

static Expression *parse_expression(Parser *parser);

void advance(Parser *parser) {
    parser->curr_token = next_token(parser->lexer);
    print_token(parser->curr_token);
}

void expect(Parser *parser, TokenType expected_type) {
    if (parser->curr_token.type != expected_type) {
        printf("Error: Unexpected token, expected %d but got %d.\n", expected_type, parser->curr_token.type);
        exit(1);
    }
    advance(parser);
}

static UNARY_OP token_to_op(TokenType token_type) {
    switch (token_type) {
        case TOK_NEG:           return OP_NEG;
        case TOK_UNARY_COMPL:   return OP_COMPL;
        case TOK_UNARY_NOT:     return OP_NOT;
        default:                return OP_FAILURE;
    }
}

static BINARY_OP token_to_bin(TokenType token_type) {
    switch (token_type) {
        case TOK_NEG:           return BIN_NEG;
        case TOK_ADD:           return BIN_ADD;
        case TOK_MULTIPLY:      return BIN_MULTIPLY;
        case TOK_DIVIDE:        return BIN_DIVIDE;
        case TOK_MOD:           return BIN_MOD;
        case TOK_AND:           return BIN_AND;
        case TOK_OR:            return BIN_OR;
        case TOK_EQ:            return BIN_EQ;
        case TOK_NEQ:           return BIN_NEQ;
        case TOK_LT:            return BIN_LT;
        case TOK_LTE:           return BIN_LTE;
        case TOK_GT:            return BIN_GT;
        case TOK_GTE:           return BIN_GTE;
        default:                return BIN_FAILURE;
    }
}

static Expression *parse_factor(Parser *parser) {
    UNARY_OP op = token_to_op(parser->curr_token.type);

    if (parser->curr_token.type == TOK_OPAREN) {
        advance(parser);
        Expression *expr = parse_expression(parser);
        expect(parser, TOK_CPAREN);
        return expr;
    }
    
    else if (op != OP_FAILURE) {
        advance(parser);
        Expression *factor = parse_factor(parser);
        Expression *unop = (Expression *) malloc(sizeof(Expression));
        unop->type = EXPR_UNOP;
        unop->lterm = factor;
        unop->un_op = op;
        return unop;
    }

    else if (parser->curr_token.type == TOK_INT_LIT) {
        Expression *expr = (Expression *) malloc(sizeof(Expression));
        expr->type = EXPR_CONST;
        expr->int_val = parser->curr_token.int_val;
        advance(parser);
        return expr;
    }

    else {
        printf("Error: Invalid expression.\n");
        exit(1);
    }
}

static Expression *parse_term(Parser *parser) {
    Expression *curr_expr = parse_factor(parser);

    while (parser->curr_token.type == TOK_MULTIPLY || parser->curr_token.type == TOK_DIVIDE || parser->curr_token.type == TOK_MOD) {
        BINARY_OP op = token_to_bin(parser->curr_token.type);
        advance(parser);
        Expression *next_factor = parse_factor(parser);

        Expression *new_expr = (Expression *) malloc(sizeof(Expression));
        new_expr->type = EXPR_BINOP;
        new_expr->lterm = curr_expr;
        new_expr->bin_op = op;
        new_expr->rterm = next_factor;

        curr_expr = new_expr;
    }

    return curr_expr;
}

static Expression *parse_additive_expression(Parser *parser) {
    Expression *curr_expr = parse_term(parser);

    while (parser->curr_token.type == TOK_ADD || parser->curr_token.type == TOK_NEG) {
        BINARY_OP op = token_to_bin(parser->curr_token.type);
        advance(parser);
        Expression *next_term = parse_term(parser);

        Expression *new_expr = (Expression *) malloc(sizeof(Expression));
        new_expr->type = EXPR_BINOP;
        new_expr->lterm = curr_expr;
        new_expr->bin_op = op;
        new_expr->rterm = next_term;

        curr_expr = new_expr;
    }

    return curr_expr;
}

static Expression *parse_relational_expression(Parser *parser) {
    Expression *curr_expr = parse_additive_expression(parser);

    while (parser->curr_token.type == TOK_LT || parser->curr_token.type == TOK_GT || parser->curr_token.type == TOK_LTE || parser->curr_token.type == TOK_GTE) {
        BINARY_OP op = token_to_bin(parser->curr_token.type);
        advance(parser);
        Expression *next_term = parse_additive_expression(parser);

        Expression *new_expr = (Expression *) malloc(sizeof(Expression));
        new_expr->type = EXPR_BINOP;
        new_expr->lterm = curr_expr;
        new_expr->bin_op = op;
        new_expr->rterm = next_term;

        curr_expr = new_expr;
    }

    return curr_expr;
}

static Expression *parse_equality_expression(Parser *parser) {
    Expression *curr_expr = parse_relational_expression(parser);

    while (parser->curr_token.type == TOK_EQ || parser->curr_token.type == TOK_NEQ) {
        BINARY_OP op = token_to_bin(parser->curr_token.type);
        advance(parser);
        Expression *next_term = parse_relational_expression(parser);

        Expression *new_expr = (Expression *) malloc(sizeof(Expression));
        new_expr->type = EXPR_BINOP;
        new_expr->lterm = curr_expr;
        new_expr->bin_op = op;
        new_expr->rterm = next_term;

        curr_expr = new_expr;
    }

    return curr_expr;
}

static Expression *parse_log_and_expression(Parser *parser) {
    Expression *curr_expr = parse_equality_expression(parser);

    while (parser->curr_token.type == TOK_AND) {
        BINARY_OP op = token_to_bin(parser->curr_token.type);
        advance(parser);
        Expression *next_term = parse_equality_expression(parser);

        Expression *new_expr = (Expression *) malloc(sizeof(Expression));
        new_expr->type = EXPR_BINOP;
        new_expr->lterm = curr_expr;
        new_expr->bin_op = op;
        new_expr->rterm = next_term;

        curr_expr = new_expr;
    }

    return curr_expr;
}

static Expression *parse_expression(Parser *parser) {
    Expression *curr_expr = parse_log_and_expression(parser);

    while (parser->curr_token.type == TOK_OR) {
        BINARY_OP op = token_to_bin(parser->curr_token.type);
        advance(parser);
        Expression *next_term = parse_log_and_expression(parser);

        Expression *new_expr = (Expression *) malloc(sizeof(Expression));
        new_expr->type = EXPR_BINOP;
        new_expr->lterm = curr_expr;
        new_expr->bin_op = op;
        new_expr->rterm = next_term;

        curr_expr = new_expr;
    }

    return curr_expr;
}

static Statement *parse_statement(Parser *parser) {
    Statement *s = (Statement *) malloc(sizeof(Statement));

    if (parser->curr_token.type == TOK_KEYW_RETURN) {
        s->type = STMT_RETURN;
        advance(parser);
        s->expr = parse_expression(parser);
        expect(parser, TOK_SEMI);
        return s;
    }
    
    else {
        printf("Error: Invalid statement.\n");
        exit(1);
    }
}

static Function *parse_function(Parser *parser) {
    Function *func = (Function *) malloc(sizeof(Function));

    expect(parser, TOK_KEYW_INT);
    if (parser->curr_token.type == TOK_IDENTIFIER) {
        strncpy(func->name, parser->curr_token.text, 63);
        func->name[63] = '\0'; // safety
        advance(parser);

        expect(parser, TOK_OPAREN);
        expect(parser, TOK_CPAREN);

        expect(parser, TOK_OBRACE);

        func->stmt = parse_statement(parser);

        expect(parser, TOK_CBRACE);

        return func;
    }

    else {
        printf("Error: Unexpected identifier.\n");
        exit(1);
    }
}

Program *parse_program(Parser *parser) {
    Program *prog = (Program *) malloc(sizeof(Program));

    prog->fnctn = parse_function(parser);
    
    expect(parser, TOK_END_OF_FILE);

    return prog;
}




// PRINTING
static void print_indent(int level) {
    for (int i = 0; i < level; i++) {
        printf("  "); // 2 spaces per level
    }
}

static void print_expression_helper(Expression *expr, int level) {
    print_indent(level);
    if (expr->type == EXPR_CONST) {
        printf("Int<%ld>", expr->int_val);
    }

    else if (expr->type == EXPR_UNOP) {
        printf("UNARY<%ld ", (long) expr->un_op);
        print_expression_helper(expr->lterm, 0);
        printf(" >");
    }

    else if (expr->type == EXPR_BINOP) {
        printf("BIN< ");
        print_expression_helper(expr->lterm, 0);
        printf(" %ld ", (long) expr->bin_op);
        print_expression_helper(expr->rterm, 0);
        printf(" >");
    }

    else {
        printf("Unknown Expression.\n");
    }
}

void print_expression(Expression *expr, int level) {
    print_expression_helper(expr, level);
    printf("\n");
}

void print_statement(Statement *stmt, int level) {
    print_indent(level);
    if (stmt->type == STMT_RETURN) {
        printf("Return\n");
        print_expression(stmt->expr, level+1);
    }

    else {
        printf("Unknown Statement.\n");
    }
}

void print_function(Function *func, int level) {
    print_indent(level);
    printf("Function: %s\n", func->name);
    print_statement(func->stmt, level+1);
}

void print_program(Program *prog, int level) {
    print_indent(level);
    printf("Program\n");
    print_function(prog->fnctn, level+1);
}
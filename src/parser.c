#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

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
        case TOK_UNARY_NEG:     return OP_NEG;
        case TOK_UNARY_COMPL:   return OP_COMPL;
        case TOK_UNARY_NOT:     return OP_NOT;
        default:                return OP_FAILURE;
    }
}

Program *parse_program(Parser *parser) {
    Program *prog = (Program *) malloc(sizeof(Program));

    prog->fnctn = parse_function(parser);
    
    expect(parser, TOK_END_OF_FILE);

    return prog;
}

Function *parse_function(Parser *parser) {
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

Statement *parse_statement(Parser *parser) {
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

Expression *parse_expression(Parser *parser) {
    Expression *expr = (Expression *) malloc(sizeof(Expression));
    
    if (parser->curr_token.type == TOK_INT_LIT) {
        expr->type = EXPR_INT_LIT;
        expr->val = parser->curr_token.int_val;
        advance(parser);
        return expr;
    }
    
    UNARY_OP unary_op = token_to_op(parser->curr_token.type);

    if (unary_op != OP_FAILURE) {
        advance(parser);
        expr->type = EXPR_UNARY;
        expr->op = unary_op;
        expr->operand = parse_expression(parser);
        return expr;
    }

    else {
        printf("Error: Invalid expression.\n");
        exit(1);
    }
}




// PRINTING
static void print_indent(int level) {
    for (int i = 0; i < level; i++) {
        printf("  "); // 2 spaces per level
    }
}

void print_expression(Expression *expr, int level) {
    print_indent(level);
    if (expr->type == EXPR_INT_LIT) {
        printf("Int<%ld>\n", expr->val);
    }

    else if (expr->type == EXPR_UNARY) {
        printf("UNARY<%ld>\n", (long) expr->op);
        print_expression(expr->operand, level+1);
    }

    else {
        printf("Unknown Expression.\n");
    }
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
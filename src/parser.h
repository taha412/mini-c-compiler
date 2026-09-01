#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef struct Expression Expression;

typedef enum STMT_TYPE {
    STMT_RETURN
} STMT_TYPE;

typedef enum EXPR_TYPE {
    EXPR_INT_LIT,
    EXPR_UNARY
} EXPR_TYPE;

typedef enum UNARY_OP {
    OP_NEG,
    OP_COMPL,
    OP_NOT,
    OP_FAILURE // exists to indicate token has no corresponding unary operator in token_to_op
} UNARY_OP;

typedef struct Parser {
    Lexer *lexer;
    Token curr_token;
} Parser;

typedef struct Expression {
    EXPR_TYPE type;
    int64_t val;
    UNARY_OP op;
    Expression *operand;
} Expression;

typedef struct Statement {
    STMT_TYPE type;
    Expression *expr;
} Statement;

typedef struct Function {
    char name[64];
    Statement *stmt;
} Function;

typedef struct Program {
    Function *fnctn;
} Program;

Program     *parse_program(Parser *parser);
Function    *parse_function(Parser *parser);
Statement   *parse_statement(Parser *parser);
Expression  *parse_expression(Parser *parser);

void advance(Parser *parser);
void expect(Parser *parser, TokenType expected_type);

Program     *parse_program(Parser *parser);
Function    *parse_function(Parser *parser);
Statement   *parse_statement(Parser *parser);
Expression  *parse_expression(Parser *parser);

void print_expression(Expression *expr, int level);
void print_statement(Statement *stmt, int level);
void print_function(Function *func, int level);
void print_program(Program *prog, int level);

#endif
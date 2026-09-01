#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef struct Expression Expression;

typedef enum STMT_TYPE {
    STMT_RETURN
} STMT_TYPE;

typedef enum EXPR_TYPE {
    EXPR_BINOP,
    EXPR_UNOP,
    EXPR_CONST
} EXPR_TYPE;

typedef enum UNARY_OP {
    OP_NEG,
    OP_COMPL,
    OP_NOT,
    OP_FAILURE // exists to indicate token has no corresponding unary operator in token_to_op
} UNARY_OP;

typedef enum BINARY_OP {
    BIN_NEG,
    BIN_ADD,
    BIN_MULTIPLY,
    BIN_DIVIDE,
    BIN_MOD,
    BIN_AND,
    BIN_OR,
    BIN_EQ,
    BIN_NEQ,
    BIN_LT,
    BIN_LTE,
    BIN_GT,
    BIN_GTE,
    BIN_FAILURE // exists to indicate token has no corresponding binary operator in token_to_bin
} BINARY_OP;

typedef struct Parser {
    Lexer *lexer;
    Token curr_token;
} Parser;

typedef struct Expression {
    EXPR_TYPE type;
    Expression *lterm;
    Expression *rterm;
    BINARY_OP bin_op;
    UNARY_OP un_op;
    int64_t int_val;
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

Program *parse_program(Parser *parser);

void advance(Parser *parser);
void expect(Parser *parser, TokenType expected_type);

void print_expression(Expression *expr, int level);
void print_statement(Statement *stmt, int level);
void print_function(Function *func, int level);
void print_program(Program *prog, int level);

#endif
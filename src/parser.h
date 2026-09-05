#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum BLCKITEM_TYPE {
    BLCKITEM_STMT,
    BLCKITEM_DECL
} BLCKITEM_TYPE;

typedef enum STMT_TYPE {
    STMT_BLOCK,
    STMT_RETURN,
    STMT_EXPR,
    STMT_COND,
    STMT_WHILE,
    STMT_DO_WHILE,
    STMT_BREAK,
    STMT_CONT
} STMT_TYPE;

typedef enum DECL_TYPE {
    DECL_INT
} DECL_TYPE;

typedef enum EXPR_TYPE {
    EXPR_BINOP,
    EXPR_UNOP,
    EXPR_CONST,
    EXPR_ASS,
    EXPR_VAR,
    EXPR_TERNARY
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

typedef struct Expression { // TODO: Add a union to reduce memory usage
    EXPR_TYPE type;
    union {
        struct Expression *lterm;
        struct Expression *term_cond;
    };
    union {
        struct Expression *rterm;
        struct Expression *term_one;
    };
    struct Expression *term_two;
    BINARY_OP bin_op;
    UNARY_OP un_op;
    int64_t int_val;
    char text[64];
    int resolved_offset;
} Expression;

typedef struct Statement {
    STMT_TYPE type;
    Expression *expr;
    union {                             // occupy same memory since will not be used at the same time
        struct BlockItem *block_head;   // type STMT_BLOCK
        struct Statement *if_stmt;      // type STMT_COND
        struct Statement *loop_stmt;      // type STMT_COND
    };
    struct Statement *else_stmt;
} Statement;

typedef struct Declaration {
    DECL_TYPE type;
    char name[64];
    Expression *expr;
    int resolved_offset;
    
} Declaration;

typedef struct BlockItem {
    BLCKITEM_TYPE type;
    union {
        Statement *stmt;
        Declaration *decl;
    };
    struct BlockItem *next;
} BlockItem;

typedef struct Function {
    char name[64];
    Statement *stmt;
    int frame_size;
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
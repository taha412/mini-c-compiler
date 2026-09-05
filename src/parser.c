#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lexer.h"
#include "parser.h"

static Expression *parse_expression(Parser *parser);
static Statement *parse_statement_block(Parser *parser);

static char *unop_to_text(UNARY_OP symb) {
    switch (symb) {
        case OP_NEG:        return "-";
        case OP_COMPL:      return "~";
        case OP_NOT:        return "!";
        case OP_FAILURE:    return "NOT A UNOP SYMBOL";
        default:            return "UNKNOWN UNOP SYMBOL";
    }
}

static char *binop_to_text(BINARY_OP symb) {
    switch (symb) {
        case BIN_ADD:       return "+";
        case BIN_NEG:       return "-";
        case BIN_MULTIPLY:  return "*";
        case BIN_DIVIDE:    return "/";
        case BIN_MOD:       return "%%";
        case BIN_AND:       return "&&";
        case BIN_OR:        return "||";
        case BIN_EQ:        return "==";
        case BIN_NEQ:       return "!=";
        case BIN_LT:        return "<";
        case BIN_LTE:       return "<=";
        case BIN_GT:        return "<";
        case BIN_GTE:       return ">=";
        case BIN_FAILURE:   return "NOT A BINOP SYMBOL";
        default:            return "UNKNOWN BINOP SYMBOL";
    }
}

void advance(Parser *parser) {
    parser->curr_token = next_token(parser->lexer);
    print_token(parser->curr_token);
}

void expect(Parser *parser, TokenType expected_type) {
    if (parser->curr_token.type != expected_type) {
        printf("Error: Unexpected token, expected %s but got %s.\n", token_type_to_string(expected_type), token_type_to_string(parser->curr_token.type));
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

    else if (parser->curr_token.type == TOK_IDENTIFIER) { // variable
        Expression *expr = (Expression *) malloc(sizeof(Expression));
        expr->type = EXPR_VAR;
        strncpy(expr->text, parser->curr_token.text, 63);
        expr->text[63] = '\0'; //safety
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

static Expression *parse_log_or_expression(Parser *parser) {
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

// using recursion instead of a while loop because ternary is right associative instead of left associative
static Expression *parse_cond_expression(Parser *parser) {
    Expression *curr_expr = parse_log_or_expression(parser);
    if (parser->curr_token.type == TOK_QMARK) {
        advance(parser);
        Expression *term_one = parse_expression(parser);
        expect(parser, TOK_COLON);
        Expression *term_two = parse_cond_expression(parser); // could use parse_expression here to enable assignment here
        Expression *new_expr = (Expression *) malloc(sizeof(Expression));
        new_expr->type = EXPR_TERNARY;
        new_expr->term_cond = curr_expr;
        new_expr->term_one = term_one;
        new_expr->term_two = term_two;

        curr_expr = new_expr;
    }

    return curr_expr;
}

static Expression *parse_expression(Parser *parser) {
    Expression *curr_expr = parse_cond_expression(parser);

    if (parser->curr_token.type == TOK_ASS) {
        if (curr_expr->type != EXPR_VAR) {
            printf("Error: Cannout assign a value to this expression\n");
            exit(1);
        }
        // create assignment expression
        Expression *next_term = (Expression *) malloc(sizeof(Expression));
        next_term->type = EXPR_ASS;
        next_term->lterm = curr_expr;
        advance(parser);
        next_term->rterm = parse_expression(parser);
        return next_term;
    }

    return curr_expr;
}

static Statement *parse_statement(Parser *parser) {
    if (parser->curr_token.type == TOK_KEYW_RETURN) {
        Statement *s = (Statement *) calloc(1, sizeof(Statement));
        s->type = STMT_RETURN;
        advance(parser);
        s->expr = parse_expression(parser);
        expect(parser, TOK_SEMI);
        return s;
    }

    else if (parser->curr_token.type == TOK_OBRACE) {
        return parse_statement_block(parser);
    }

    else if (parser->curr_token.type == TOK_IF) {
        Statement *s = (Statement *) calloc(1, sizeof(Statement));
        s->type = STMT_COND;
        advance(parser);

        expect(parser, TOK_OPAREN);
        s->expr = parse_expression(parser);
        expect(parser, TOK_CPAREN);

        s->if_stmt = parse_statement(parser);

        if (parser->curr_token.type == TOK_ELSE) {
            advance(parser);
            s->else_stmt = parse_statement(parser);
        }
        return s;
    }

    else if (parser->curr_token.type == TOK_FOR) {
        //TODO
    }

    else if (parser->curr_token.type == TOK_WHILE) {
        Statement *s = (Statement *) calloc(1, sizeof(Statement));
        s->type = STMT_WHILE;
        advance(parser);
        
        expect(parser, TOK_OPAREN);
        s->expr = parse_expression(parser);
        expect(parser, TOK_CPAREN);

        s->loop_stmt = parse_statement(parser);
        return s;
    }

    else if (parser->curr_token.type == TOK_DO) {
        Statement *s = (Statement *) calloc(1, sizeof(Statement));
        s->type = STMT_DO_WHILE;
        advance(parser);

        s->loop_stmt = parse_statement(parser);

        expect(parser, TOK_WHILE);
        expect(parser, TOK_OPAREN);
        s->expr = parse_expression(parser);
        expect(parser, TOK_CPAREN);
        expect(parser, TOK_SEMI);

        return s;
    }

    else if (parser->curr_token.type == TOK_CONT) {
        Statement *s = (Statement *) calloc(1, sizeof(Statement));
        s->type = STMT_CONT;
        return s;
    }

    else if (parser->curr_token.type == TOK_BREAK) {
        Statement *s = (Statement *) calloc(1, sizeof(Statement));
        s->type = STMT_BREAK;
        return s;
    }

    // try to parse it as an expression
    Statement *s = (Statement *) calloc(1, sizeof(Statement));
    s->type = STMT_EXPR;
    s->expr = parse_expression(parser);
    expect(parser, TOK_SEMI);
    return s;

}

static Declaration *parse_declaration(Parser *parser) {
    if (parser->curr_token.type == TOK_KEYW_INT) { // safety
        Declaration *d = (Declaration *) calloc(1, sizeof(Declaration));
        d->type = DECL_INT;
        advance(parser);
        if (parser->curr_token.type != TOK_IDENTIFIER) {
            printf("Error: Invalid variable name.");
            exit(1);
        }
        strncpy(d->name, parser->curr_token.text, 63);
        d->name[63] = '\0'; // safety
        advance(parser);
        d->expr = NULL;
        if (parser->curr_token.type == TOK_ASS) {
            advance(parser);
            d->expr = parse_expression(parser);
        }
        expect(parser, TOK_SEMI);
        return d;
    } else {
        printf("Error: Unrecognized declaration\n");
        exit(1);
    }
}

static BlockItem *parse_block_item(Parser *parser) {
    BlockItem *bi = (BlockItem *) calloc(1, sizeof(BlockItem));

    if (parser->curr_token.type == TOK_KEYW_INT) {
        bi->type = BLCKITEM_DECL;
        bi->decl = parse_declaration(parser);
        return bi;
    }

    else {
        bi->type = BLCKITEM_STMT;
        bi->stmt = parse_statement(parser);
        return bi;
    }
}

static Statement *parse_statement_block(Parser *parser) {
    expect(parser, TOK_OBRACE);
    Statement *head = (Statement *) calloc(1, sizeof(Statement));
    head->type = STMT_BLOCK;
    BlockItem *curr = NULL;
    BlockItem *new = NULL;
    while (parser->curr_token.type != TOK_CBRACE && parser->curr_token.type != TOK_END_OF_FILE) {
        new = parse_block_item(parser);
        if (curr == NULL) {
            head->block_head = new;
        } else {
            curr->next = new;
        }
        curr = new;
    }

    expect(parser, TOK_CBRACE);

    return head;
}

static Function *parse_function(Parser *parser) {
    Function *func = (Function *) calloc(1, sizeof(Function));

    expect(parser, TOK_KEYW_INT);
    if (parser->curr_token.type == TOK_IDENTIFIER) {
        strncpy(func->name, parser->curr_token.text, 63);
        func->name[63] = '\0'; // safety
        advance(parser);

        expect(parser, TOK_OPAREN);
        expect(parser, TOK_CPAREN);

        func->stmt = parse_statement_block(parser);

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

void print_block_item(BlockItem *bi, int level);

static void print_indent(int level) {
    for (int i = 0; i < level; i++) {
        printf("  "); // 2 spaces per level
    }
}

void print_expression(Expression *expr, int level) {
    print_indent(level);
    if (expr->type == EXPR_CONST) {
        printf("Int<%ld>", expr->int_val);
    }

    else if (expr->type == EXPR_UNOP) {
        printf("UNARY<%s ", unop_to_text(expr->un_op));
        print_expression(expr->lterm, 0);
        printf(" >");
    }

    else if (expr->type == EXPR_BINOP) {
        printf("BIN< ");
        print_expression(expr->lterm, 0);
        printf(" %s ", binop_to_text(expr->bin_op));
        print_expression(expr->rterm, 0);
        printf(" >");
    }

    else if (expr->type == EXPR_VAR) {
        printf("VAR<%s>", expr->text);
    }

    else if (expr->type == EXPR_ASS) {
        printf("ASS< %s = ", expr->lterm->text);
        print_expression(expr->rterm, 0);
        printf(" >");
    }

    else if (expr->type == EXPR_TERNARY) {
        printf("TERNARY< ");
        print_expression(expr->term_cond, 0);
        printf(" ? ");
        print_expression(expr->term_one, 0);
        printf(" : ");
        print_expression(expr->term_two, 0);
        printf(" >");
    }

    else {
        printf("Unknown Expression.\n");
    }
}

void print_declaration(Declaration *decl, int level) {
    print_indent(level);
    if (decl->type == DECL_INT) {
        printf("int %s", decl->name);
        if (decl->expr != NULL) {
            printf(" = ");
            print_expression(decl->expr, 0);
        }
    }
}

void print_block_item(BlockItem *bi, int level) {
    if (bi->type == BLCKITEM_STMT) {
        print_statement(bi->stmt, level);
    } else {
        print_declaration(bi->decl, level);
    }
    printf("\n");
    if (bi->next != NULL) {
        print_block_item(bi->next, level);
    }
}

void print_statement(Statement *stmt, int level) {
    print_indent(level);

    switch (stmt->type) {
        case STMT_RETURN:
            printf("Return ");
            print_expression(stmt->expr, 0);
            break;
        case STMT_BLOCK:
            printf("BEGIN BLOCK\n");
            print_block_item(stmt->block_head, level+1);
            print_indent(level);
            printf("BLOCK END");
            break;
        case STMT_EXPR:
            print_expression(stmt->expr, 0);
            break;
        case STMT_COND:
            printf("IF ( ");
            print_expression(stmt->expr, 0);
            printf(" )\n");
            print_statement(stmt->if_stmt, level);
            printf("\n");
            if (stmt->else_stmt != NULL) {
                print_indent(level);
                printf("ELSE\n");
                print_statement(stmt->else_stmt, level+1);
                printf("\n");
            }
            break;
        case STMT_WHILE:
            printf("WHILE ( ");
            print_expression(stmt->expr, 0);
            printf(" )\n");
            print_statement(stmt->loop_stmt, level+1);
            break;
        case STMT_DO_WHILE:
            printf("DO\n");
            print_statement(stmt->loop_stmt, level+1);
            printf("\n");
            print_indent(level);
            printf("WHILE (");
            print_expression(stmt->expr, 0);
            printf(" )");
            break;
        default:
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
    printf("\n");
}
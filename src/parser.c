#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lexer.h"
#include "parser.h"

static Expression *parse_expression(Parser *parser);
static Statement *parse_statement_block(Parser *parser);
static BlockItem *parse_block_item(Parser *parser);

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

    else if (parser->curr_token.type == TOK_IDENTIFIER) { // variable or function
        Expression *expr = (Expression *) malloc(sizeof(Expression));
        strncpy(expr->text, parser->curr_token.text, 63);
        expr->text[63] = '\0'; //safety
        advance(parser);
        if (parser->curr_token.type == TOK_OPAREN) {
            expr->type = EXPR_FUNC;
            //TODO FUNCTION STUFF
        } else {
            expr->type = EXPR_VAR;
        }
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
        Statement *s = (Statement *) calloc(1, sizeof(Statement));
        s->type = STMT_FOR;
        advance(parser);

        expect(parser, TOK_OPAREN);

        if (parser->curr_token.type != TOK_SEMI) {
            s->init = parse_block_item(parser);
            // error check only works as long as BlockItem can only be a declaration or a statement
            // since it uses short-circuiting in the && operator
            if (s->init->type != BLCKITEM_DECL && s->init->stmt->type != STMT_EXPR) { 
                printf("Error: For loop initialization can only be an expression or declaration");
                exit(1);
            }
        } else {
            s->init = NULL;
            expect(parser, TOK_SEMI);
        }

        if (parser->curr_token.type != TOK_SEMI) {
            s->cond = parse_expression(parser);
        } else {
            s->cond = NULL;
        }
        expect(parser, TOK_SEMI);

        if (parser->curr_token.type != TOK_CPAREN) {
            s->post = parse_expression(parser);
        } else {
            s->post = NULL;
        }
        expect(parser, TOK_CPAREN);

        s->loop_stmt = parse_statement(parser);
        return s;
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
        advance(parser);
        expect(parser, TOK_SEMI);
        return s;
    }

    else if (parser->curr_token.type == TOK_BREAK) {
        Statement *s = (Statement *) calloc(1, sizeof(Statement));
        s->type = STMT_BREAK;
        advance(parser);
        expect(parser, TOK_SEMI);
        return s;
    }

    else if (parser->curr_token.type == TOK_SEMI) {
        Statement *s = (Statement *) calloc(1, sizeof(Statement));
        s->type = STMT_NULL;
        advance(parser);
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
    BlockItem *new_block = NULL;
    while (parser->curr_token.type != TOK_CBRACE && parser->curr_token.type != TOK_END_OF_FILE) {
        new_block = parse_block_item(parser);
        if (curr == NULL) {
            head->block_head = new_block;
        } else {
            curr->next = new_block;
        }
        curr = new_block;
    }

    expect(parser, TOK_CBRACE);

    return head;
}

static Parameter *parse_parameters(Parser *parser, int *para_count) {
    *para_count = 0;

    if (parser->curr_token.type == TOK_VOID) {
        advance(parser);
        return NULL;
    }

    if (parser->curr_token.type == TOK_CPAREN) {
        return NULL;
    }

    Parameter *head = NULL;
    Parameter *curr = NULL;
    while (parser->curr_token.type != TOK_END_OF_FILE) {
        if (parser->curr_token.type == TOK_KEYW_INT) { // change later to accept multiple types
            advance(parser);
            if (parser->curr_token.type != TOK_IDENTIFIER) {
                printf("Error: Invalid parameter name in function declarationn\n");
                exit(1);
            }

            Parameter *para = (Parameter *) calloc(1, sizeof(Parameter));
            para->type = DECL_INT; // change later to accept multiple types
            strncpy(para->name, parser->curr_token.text, 63);
            para->name[63] = '\0'; // for safety

            if (head == NULL) {
                head = curr = para;
            } else {
                curr->next = para;
                curr = para;
            }
            (*para_count)++;
            advance(parser);

            if (parser->curr_token.type == TOK_COMMA) {
                advance(parser);
                continue; // just in case
            } else {
                break;
            }
        } else {
            printf("Error: Invalid function parameter\n");
            exit(1);
        }
    }
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
        func->para = parse_parameters(parser, &(func->para_count));
        expect(parser, TOK_CPAREN);

        if (parser->curr_token.type == TOK_SEMI) {
            func->stmt = NULL;
            advance(parser);
        } else {
            func->stmt = parse_statement_block(parser);
        }

        return func;
    }

    else {
        printf("Error: Unexpected identifier.\n");
        exit(1);
    }
}

static TopLevelItem *parse_top_lvl_item(Parser *parser) { // to allow implementation of gloval variables in the future
    TopLevelItem *top = (TopLevelItem *) calloc(1, sizeof(TopLevelItem));

    top->type = TOPLVL_FNCTN;
    top->fnctn = parse_function(parser);

    return top;
}

Program *parse_program(Parser *parser) {
    Program *prog = (Program *) calloc(1, sizeof(Program));
    TopLevelItem *curr = NULL;

    while (parser->curr_token.type != TOK_END_OF_FILE) {
        TopLevelItem *new_item = parse_top_lvl_item(parser);
        if (prog->top == NULL) {
            prog->top = curr = new_item;
        } else {
            curr->next = new_item;
            curr = new_item;
        }
    }
    
    expect(parser, TOK_END_OF_FILE); // just in case

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

void print_block_item_helper(BlockItem *bi, int level) {
    if (bi->type == BLCKITEM_STMT) {
        print_statement(bi->stmt, level);
    } else {
        print_declaration(bi->decl, level);
    }
}

void print_block_item(BlockItem *bi, int level) {
    print_block_item_helper(bi, level);
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
            if (stmt->block_head != NULL) {
                print_block_item(stmt->block_head, level+1);
            }
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
            print_statement(stmt->if_stmt, level+1);
            if (stmt->else_stmt != NULL) {
                print_indent(level);
                printf("ELSE\n");
                print_statement(stmt->else_stmt, level+1);
            }
            break;
        case STMT_FOR:
            printf("FOR ( ");
            if (stmt->init != NULL) {
                print_block_item_helper(stmt->init, 0);
            }
            printf(" ; ");
            if (stmt->cond != NULL) {
                print_expression(stmt->cond, 0);
            }
            printf(" ; ");
            if (stmt->post != NULL) {
                print_expression(stmt->post, 0);
            }
            printf(" )\n");
            print_statement(stmt->loop_stmt, level+1);
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
        case STMT_BREAK:
            printf("BREAK");
            break;
        case STMT_CONT:
            printf("CONTINUE");
            break;
        case STMT_NULL:
            break;
        default:
            printf("Unknown Statement.\n");
    }
}

void print_function(Function *func, int level) {
    print_indent(level);
    printf("Function: %s\n", func->name);
    print_statement(func->stmt, level+1);
    printf("\n");
}

void print_top_level_item(TopLevelItem *tli, int level) {
    if (tli->type == TOPLVL_FNCTN) {
        print_function(tli->fnctn, level);
    }
}

void print_program(Program *prog, int level) {
    print_indent(level);
    printf("Program\n");
    TopLevelItem *curr = prog->top;
    while (curr != NULL) {
        print_top_level_item(curr, level+1);
        curr = curr->next;
    }
    printf("\n");
}
#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Lexer {
    const char *source;
    size_t pos;
} Lexer;

typedef enum TokenType {
    TOK_OBRACE,
    TOK_CBRACE,
    TOK_OPAREN,
    TOK_CPAREN,
    TOK_SEMI,
    TOK_IDENTIFIER,
    TOK_INT_LIT,
    TOK_END_OF_FILE,
    TOK_KEYW_INT,
    TOK_KEYW_RETURN
} TokenType;

typedef struct Token {
    TokenType type;
    int64_t int_val; // Using 64-bit system
    char text[64];
} Token;

const char* token_type_to_string(TokenType type);
void print_token(Token t);
char* read_file(const char* filepath);
Token next_token(Lexer *lexer);

#endif
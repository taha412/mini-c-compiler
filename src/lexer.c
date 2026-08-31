#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"

// For printing
const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOK_OBRACE:        return "OBRACE";
        case TOK_CBRACE:        return "CBRACE";
        case TOK_OPAREN:        return "OPAREN";
        case TOK_CPAREN:        return "CPAREN";
        case TOK_SEMI:          return "SEMI";
        case TOK_IDENTIFIER:    return "IDENTIFIER";
        case TOK_INT_LIT:       return "INT_LIT";
        case TOK_END_OF_FILE:   return "END_OF_FILE";
        case TOK_KEYW_INT:      return "KEYW_INT";
        case TOK_KEYW_RETURN:   return "KEYW_RETURN";
        default:            return "UNKNOWN";
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
void print_token(Token t) {
    printf("Token: %-15s", token_type_to_string(t.type));

    if (t.type == TOK_IDENTIFIER || t.type == TOK_KEYW_INT || t.type == TOK_KEYW_RETURN) {
        printf(" | name: %s\n", t.text);
    } 

    else if (t.type == TOK_INT_LIT) {
        printf(" | value: %lld\n", (long long)t.int_val); 
    } 

    else {
        printf("\n");
    }
}



char* read_file(const char* filepath) {
    FILE *file_ptr;

    file_ptr = fopen(filepath, "r");

    if (file_ptr == NULL) {
        printf("Error: Could not open file.\n");
        exit(1);
    }

    // determine size of file
    fseek(file_ptr, 0, SEEK_END);
    long file_size = ftell(file_ptr);
    rewind(file_ptr);

    // allocate required memory
    char *buffer = (char *) malloc(file_size + 1);

    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }
    
    // place file contents into buffer
    size_t bytes_read = fread(buffer, 1, file_size, file_ptr);
    buffer[bytes_read] = '\0';

    fclose(file_ptr);

    return buffer;
}

Token next_token(Lexer *lexer) {
    while (isspace((unsigned char) lexer->source[lexer->pos])) {
        lexer->pos++;
    }

    Token t = {0};

    if (lexer->source[lexer->pos] == '\0')
        t.type = TOK_END_OF_FILE;

    else if (lexer->source[lexer->pos] == '{') {
        lexer->pos++;
        t.type = TOK_OBRACE;
    }
    else if (lexer->source[lexer->pos] == '}') {
        lexer->pos++;
        t.type = TOK_CBRACE;
    }
    else if (lexer->source[lexer->pos] == '(') {
        lexer->pos++;
        t.type = TOK_OPAREN;
    }
    else if (lexer->source[lexer->pos] == ')') {
        lexer->pos++;
        t.type = TOK_CPAREN;
    }
    else if (lexer->source[lexer->pos] == ';') {
        lexer->pos++;
        t.type = TOK_SEMI;
    }
    // look for words
    else if (isalpha((unsigned char) lexer->source[lexer->pos]) || lexer->source[lexer->pos] == '_') {
        int text_pos = 0;

        // get word into t.text
        while ((isalnum(lexer->source[lexer->pos]) || lexer->source[lexer->pos] == '_') && text_pos < 63) {
            t.text[text_pos] = lexer->source[lexer->pos];
            text_pos++;
            lexer->pos++;
        }
        t.text[text_pos] = '\0';

        if (strcmp(t.text, "int") == 0) {
            t.type = TOK_KEYW_INT;
        }
        else if (strcmp(t.text, "return") == 0) {
            t.type = TOK_KEYW_RETURN;
        } else {
            t.type = TOK_IDENTIFIER;
        }
    }
    // look for numbers
    else if (isdigit((unsigned char) lexer->source[lexer->pos])) {
        // get num into int_val
        t.int_val = 0;
        while (isdigit((unsigned char) lexer->source[lexer->pos])) {
            t.int_val *= 10;
            t.int_val += lexer->source[lexer->pos] - '0';
            lexer->pos++;
        }
        t.type = TOK_INT_LIT;

    }
    else {
        printf("Error: Unrecognized character '%c'\n", lexer->source[lexer->pos]);
        exit(1);
    }

    return t;
}
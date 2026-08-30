#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>

typedef struct Lexer {
    const char *source;
    size_t pos;
} Lexer;

typedef enum TokenType {
    OBRACE,
    CBRACE,
    OPAREN,
    CPAREN,
    SEMI,
    INDENTIFIER,
    INT_LIT,
    END_OF_FILE,
    KEYW_INT,
    KEYW_RETURN
} TokenType;

typedef struct Token {
    TokenType type;
    int64_t int_val; // Using 64-bit system
    char text[64];
} Token;

//For printing
const char* token_type_to_string(TokenType type) {
    switch (type) {
        case OBRACE:        return "OBRACE";
        case CBRACE:        return "CBRACE";
        case OPAREN:        return "OPAREN";
        case CPAREN:        return "CPAREN";
        case SEMI:          return "SEMI";
        case INDENTIFIER:   return "INDENTIFIER";
        case INT_LIT:       return "INT_LIT";
        case END_OF_FILE:   return "END_OF_FILE";
        case KEYW_INT:      return "KEYW_INT";
        case KEYW_RETURN:   return "KEYW_RETURN";
        default:            return "UNKNOWN";
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
void print_token(Token t) {
    printf("Token: %-15s", token_type_to_string(t.type));

    if (t.type == INDENTIFIER || t.type == KEYW_INT || t.type == KEYW_RETURN) {
        printf(" | name: %s\n", t.text);
    } 

    else if (t.type == INT_LIT) {
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

    //Determine size of file
    fseek(file_ptr, 0, SEEK_END);
    long file_size = ftell(file_ptr);
    rewind(file_ptr);

    // allocate required memory
    char *buffer = (char *) malloc(file_size + 1);

    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }
    
    //place file contents into buffer
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
        t.type = END_OF_FILE;

    else if (lexer->source[lexer->pos] == '{') {
        lexer->pos++;
        t.type = OBRACE;
    }
    else if (lexer->source[lexer->pos] == '}') {
        lexer->pos++;
        t.type = CBRACE;
    }
    else if (lexer->source[lexer->pos] == '(') {
        lexer->pos++;
        t.type = OPAREN;
    }
    else if (lexer->source[lexer->pos] == ')') {
        lexer->pos++;
        t.type = CPAREN;
    }
    else if (lexer->source[lexer->pos] == ';') {
        lexer->pos++;
        t.type = SEMI;
    }
    // look for words
    else if (isalpha((unsigned char) lexer->source[lexer->pos]) || lexer->source[lexer->pos] == '_') {
        int text_pos = 0;

        //get word into t.text
        while ((isalnum(lexer->source[lexer->pos]) || lexer->source[lexer->pos] == '_') && text_pos < 63) {
            t.text[text_pos] = lexer->source[lexer->pos];
            text_pos++;
            lexer->pos++;
        }
        t.text[text_pos] = '\0';

        if (strcmp(t.text, "int") == 0) {
            t.type = KEYW_INT;
        }
        else if (strcmp(t.text, "return") == 0) {
            t.type = KEYW_RETURN;
        } else {
            t.type = INDENTIFIER;
        }
    }
    // look for numbers
    else if (isdigit((unsigned char) lexer->source[lexer->pos])) {
        //get num into int_val
        t.int_val = 0;
        while (isdigit((unsigned char) lexer->source[lexer->pos])) {
            t.int_val *= 10;
            t.int_val += lexer->source[lexer->pos] - '0';
            lexer->pos++;
        }
        t.type = INT_LIT;

    }
    else {
        printf("Error: Unrecognized character '%c'\n", lexer->source[lexer->pos]);
        exit(1);
    }

    return t;
}
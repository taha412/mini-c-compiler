#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"

int main(int argc, char** args) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file.c>\n", args[0]);
        exit(1);
    }

    char* code = read_file(args[1]);

    Lexer lexer = {.source = code, .pos = 0};

    Parser parser = {.lexer = &lexer};

    advance(&parser);

    Program *ast = parse_program(&parser);

    printf("--- AST ---\n");
    print_program(ast, 0);
    printf("-----------\n");


    free(code);

    return 0;
}
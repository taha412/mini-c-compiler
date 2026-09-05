#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "resolve.h"

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

    FILE *out = fopen("output.s", "w");
    if (!out) {
        perror("Error: Failed to open output.s\n");
        free(code);
        return 1;
    }

    resolve_code(ast);

    generate_code(ast, out);

    fclose(out);

    free(code);

    printf("Successfully generated assemply: output.s!\n");

    return 0;
}
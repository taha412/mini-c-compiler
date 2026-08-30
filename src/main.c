#include "lexer.c"

int main(int argc, char** args) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file.c>\n", args[0]);
        exit(1);
    }

    char* code = read_file(args[1]);

    Lexer lexer = {.source = code, .pos = 0};

    Token t;

    while (t.type != END_OF_FILE) {
        t = next_token(&lexer);
        print_token(t);
    }

    free(code);

    return 0;
}
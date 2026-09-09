CC = gcc
CFLAGS = -Wall -Wextra -g
SRCS = main.c lexer.c parser.c codegen.c symtab.c resolve.c
TARGET = mini-compiler

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

test: $(TARGET)
	../tests/test.sh

clean:
	rm -f $(TARGET) my_prog output.s temp_test.c

run: $(SRCS)
	@$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)
	@./$(TARGET) $(dir)
	@gcc output.s -o output_temp
	@./output_temp; \
	STATUS=$$?; \
	echo "$$STATUS"; \
	rm -f output_temp;
# 	rm -f output.s output_temp;


.PHONY: all clean test
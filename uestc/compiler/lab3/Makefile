# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Flex and Bison files
LEX_FILE = lrlex.l
YACC_FILE = lrparser.y

# Generated files
LEX_C = lex.yy.c
YACC_C = lrparser.tab.c
YACC_H = lrparser.tab.h

# Source files
SRCS = main.c ast.c $(YACC_C) $(LEX_C)
OBJS = main.o ast.o lrparser.tab.o lex.yy.o

# Output binary
TARGET = myparser

# Default rule
all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lfl

# Compile Flex file
$(LEX_C): $(LEX_FILE)
	flex $(LEX_FILE)

# Compile Bison file
$(YACC_C) $(YACC_H): $(YACC_FILE)
	bison -d $(YACC_FILE)

# Compile C source files
main.o: main.c ast.h $(YACC_H)
	$(CC) $(CFLAGS) -c main.c

ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c

lrparser.tab.o: $(YACC_C)
	$(CC) $(CFLAGS) -c $(YACC_C)

lex.yy.o: $(LEX_C) $(YACC_H)
	$(CC) $(CFLAGS) -c $(LEX_C)

# Clean up generated and object files
clean:
	rm -f $(OBJS) $(LEX_C) $(YACC_C) $(YACC_H) $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run

CC = gcc
CFLAGS = -Wall -g

LEX_FILE = lrlex.l
YACC_FILE = lrparser.y

LEX_C = lex.yy.c
YACC_C = lrparser.tab.c
YACC_H = lrparser.tab.h

SRCS = main.c ast.c $(YACC_C) $(LEX_C)
OBJS = main.o ast.o lrparser.tab.o lex.yy.o

TARGET = parser

INPUT_DIR = ../test_cases
OUTPUT_DIR = ../test_cases_output

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lfl

$(LEX_C): $(LEX_FILE)
	flex $(LEX_FILE)

$(YACC_C) $(YACC_H): $(YACC_FILE)
	bison -d $(YACC_FILE)

main.o: main.c ast.h $(YACC_H)
	$(CC) $(CFLAGS) -c main.c

ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c

lrparser.tab.o: $(YACC_C)
	$(CC) $(CFLAGS) -c $(YACC_C)

lex.yy.o: $(LEX_C) $(YACC_H)
	$(CC) $(CFLAGS) -c $(LEX_C)

clean:
	rm -f $(OBJS) $(LEX_C) $(YACC_C) $(YACC_H) $(TARGET) 
	rm -rf $(OUTPUT_DIR)

prepare-test-dir:
	mkdir -p $(OUTPUT_DIR)

test: $(TARGET) prepare-test-dir
	@echo "Running tests..."
	@for file in $(INPUT_DIR)/*.sy; do \
		base_name=$$(basename $$file .sy); \
		output_file=$(OUTPUT_DIR)/$$base_name.out; \
		echo "Processing $$file -> $$output_file"; \
		./$(TARGET) < $$file > $$output_file || { echo "Test failed for $$file"; exit 1; }; \
	done
	@echo "All tests completed successfully."


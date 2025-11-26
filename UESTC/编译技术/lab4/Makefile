CC = gcc
CFLAGS = -Wall -g

LEX_FILE = lrlex.l
YACC_FILE = lrparser.y

LEX_C = lex.yy.c
YACC_C = lrparser.tab.c
YACC_H = lrparser.tab.h

SRCS = main.c codegen.c ast.c $(YACC_C) $(LEX_C)
OBJS = main.o codegen.o ast.o lrparser.tab.o lex.yy.o

TARGET = codegen

INPUT_DIR = test_cases
OUTPUT_DIR = ../test_cases_output

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lfl

$(LEX_C): $(LEX_FILE)
	flex $(LEX_FILE)

$(YACC_C) $(YACC_H): $(YACC_FILE)
	bison -d $(YACC_FILE)

main.o: main.c codegen.h $(YACC_H)
	$(CC) $(CFLAGS) -c main.c

codegen.o: codegen.c codegen.h ast.h
	$(CC) $(CFLAGS) -c codegen.c

ast.o: ast.c ast.h $(YACC_H)
	$(CC) $(CFLAGS) -c ast.c

lrparser.tab.o: $(YACC_C)
	$(CC) $(CFLAGS) -c $(YACC_C)

lex.yy.o: $(LEX_C) $(YACC_H)
	$(CC) $(CFLAGS) -c $(LEX_C)

clean:
	rm -f $(OBJS) $(LEX_C) $(YACC_C) $(YACC_H) $(TARGET)
	rm -rf $(OUTPUT_DIR)

prepare-output-dir:
	mkdir -p $(OUTPUT_DIR)


test: $(TARGET) prepare-output-dir
	@echo "运行测试..."
	@for file in $(INPUT_DIR)/*.c; do \
		if [ -f $$file ]; then \
			base_name=$$(basename $$file .c); \
			output_file=$(OUTPUT_DIR)/$$base_name.ll; \
			echo "处理 $$file -> $$output_file"; \
			./$(TARGET) $$file > $$output_file || { echo "测试失败: $$file"; exit 1; }; \
		fi; \
	done
	@echo "测试完成。"

.PHONY: all clean prepare-output-dir test


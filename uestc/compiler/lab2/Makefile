# 编译器和工具
CC = gcc
LEX = flex

# 目标文件名
TARGET = rd_icoding
LEX_FILE = sysy_lex.l
INPUT_DIR = ./test_cases
OUTPUT_DIR = ./test_cases_output

# 默认目标
all: $(TARGET)

# 编译目标程序
$(TARGET): rd_icoding.c lex.yy.c
	$(CC) -o $(TARGET) rd_icoding.c lex.yy.c -lfl

# 生成 lex.yy.c 文件
lex.yy.c: $(LEX_FILE)
	$(LEX) $(LEX_FILE)

# 清理生成的文件
clean:
	rm -f $(TARGET) lex.yy.c
	rm -rf $(OUTPUT_DIR)

# 创建测试输出目录
prepare-test-dir:
	mkdir -p $(OUTPUT_DIR)

# 测试目标
test: $(TARGET) prepare-test-dir
	@echo "Running tests..."
	@for file in $(INPUT_DIR)/*.sy; do \
		base_name=$$(basename $$file .sy); \
		output_file=$(OUTPUT_DIR)/$$base_name.out; \
		echo "Processing $$file -> $$output_file"; \
		./$(TARGET) < $$file > $$output_file || { echo "Test failed for $$file"; exit 1; }; \
	done
	@echo "All tests completed successfully."

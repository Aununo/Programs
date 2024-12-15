CC = gcc
LEX = flex

TARGET = main
LEX_FILE = auto_lex.l
INPUT_DIR = ./test_cases
OUTPUT_DIR = ./test_cases_output

all: $(TARGET)

$(TARGET): main.c lex.yy.c
	$(CC) -o $(TARGET) main.c lex.yy.c -lfl

lex.yy.c: $(LEX_FILE)
	$(LEX) $(LEX_FILE)

clean:
	rm -f $(TARGET) lex.yy.c
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

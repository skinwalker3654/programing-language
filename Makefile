COMPILER = compiler.c
OUTPUT = output.c
CC = gcc

TAR = main
$(TAR): $(COMPILER)
	@echo "compiling the compiler..."
	$(CC) $< -o $@

clean:
	rm -rf $(TAR)
	rm -rf $(OUTPUT)

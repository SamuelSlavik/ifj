 ##
 # @file Makefile
 # @author Michal Ľaš (xlasmi00)
 #

CC=gcc
CFLAGS=-std=c11 -pedantic -Wall -Wextra -g
PROG=./bin/test-scanner ./bin/test-dynamic_buffer ./bin/test-parser
.PHONY: clean all


all: $(PROG)
	

############################## SCANNER ###################################

./bin/scanner.o: ./src/scanner.c ./src/dynamic_buffer.c
	$(CC) $(CFLAGS) -c $< -o $@

./bin/test-scanner.o: ./src/scanner.c ./src/dynamic_buffer.c
	$(CC) $(CFLAGS) -DTESTING -c $< -o $@	

./bin/test-scanner: ./bin/test-scanner.o ./bin/dynamic_buffer.o ./bin/error.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

########################### DYNAMIC_BUFFER ###############################

./bin/dynamic_buffer.o: ./src/dynamic_buffer.c
	$(CC) $(CFLAGS) -c $< -o $@

./bin/test-dynamic_buffer.o: ./src/dynamic_buffer.c
	$(CC) $(CFLAGS) -DTESTING -c $< -o $@

./bin/test-dynamic_buffer: ./bin/test-dynamic_buffer.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

############################### STACK ####################################

./bin/stack.o: ./src/stack.c
	$(CC) $(CFLAGS) -c $< -o $@

########################### SYMBOL_TABLE #################################

./bin/htab.o: ./src/htab.c
	$(CC) $(CFLAGS) -c $< -o $@

./bin/symtable.o: ./src/symtable.c ./src/htab.c ./src/stack.c ./src/scanner.c
	$(CC) $(CFLAGS) -c $< -o $@
  
############################### ERROR #####################################

./bin/error.o: ./src/error.c
	$(CC) $(CFLAGS) -c $< -o $@

########################### PARSER #################################
./bin/test-parser: ./bin/parser.o ./bin/scanner.o ./bin/dynamic_buffer.o ./bin/error.o ./bin/expression_parser.o ./bin/stack.o ./bin/dynamic_buffer.o ./bin/dll_instruction_list.o ./bin/htab.o ./bin/symtable.o ./bin/expression_codegen.o ./bin/generator.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

./bin/parser.o: ./src/parser.c
	$(CC) $(CFLAGS) -c $< -o $@

./bin/dll_instruction_list.o: ./src/dll_instruction_list.c
	$(CC) $(CFLAGS) -c $< -o $@

############################## EXPRESSION PARSER ###################################

./bin/expression_parser.o: ./src/expression_parser.c
	$(CC) $(CFLAGS) -c $< -o $@

############################## EXPRESSION CODEGEN ###################################

./bin/expression_codegen.o: ./src/expression_codegen.c
	$(CC) $(CFLAGS) -c $< -o $@

./bin/generator.o: ./src/generator.c
	$(CC) $(CFLAGS) -c $< -o $@


############################### XXXXX #####################################

clean:
	rm -f ./bin/*

 ##
 # @file Makefile
 # @author Michal Ľaš (xlasmi00)
 #

CC=gcc
CFLAGS=-O2 -std=c11 -pedantic -Wall -Wextra
PROG=parser
.PHONY: clean all


all: $(PROG)
	

############################## SCANNER ###################################

scanner.o: scanner.c dynamic_buffer.c
	$(CC) $(CFLAGS) -c $< -o $@

########################### DYNAMIC_BUFFER ###############################

dynamic_buffer.o: dynamic_buffer.c
	$(CC) $(CFLAGS) -c $< -o $@

############################### STACK ####################################

stack.o: stack.c
	$(CC) $(CFLAGS) -c $< -o $@

########################### SYMBOL_TABLE #################################

htab.o: htab.c
	$(CC) $(CFLAGS) -c $< -o $@

symtable.o: symtable.c htab.c stack.c scanner.c
	$(CC) $(CFLAGS) -c $< -o $@
  
############################### ERROR #####################################

error.o: error.c
	$(CC) $(CFLAGS) -c $< -o $@

########################### PARSER #################################
parser: parser.o scanner.o dynamic_buffer.o error.o expression_parser.o stack.o dynamic_buffer.o dll_instruction_list.o htab.o symtable.o expression_codegen.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

parser.o: parser.c
	$(CC) $(CFLAGS) -c $< -o $@

dll_instruction_list.o: dll_instruction_list.c
	$(CC) $(CFLAGS) -c $< -o $@

############################## EXPRESSION PARSER ###################################

expression_parser.o: expression_parser.c
	$(CC) $(CFLAGS) -c $< -o $@

############################## EXPRESSION CODGEN ###################################

expression_codegen.o: expression_codegen.c
	$(CC) $(CFLAGS) -c $< -o $@

############################### XXXXX #####################################

clean:
	rm -f *.o parser

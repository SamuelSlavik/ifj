 ##
 # @file Makefile
 # @author Michal Ľaš (xlasmi00)
 #

CC=gcc
CFLAGS=-O2 -std=c11 -pedantic -Wall -Wextra
PROG=test-scanner test-dynamic_buffer test-parser

.PHONY: clean all


all: $(PROG)
	

############################## SCANNER ###################################

scanner.o: scanner.c dynamic_buffer.c
	$(CC) $(CFLAGS) -c $< -o $@

test-scanner.o: scanner.c dynamic_buffer.c
	$(CC) $(CFLAGS) -DTESTING -c $< -o $@	

test-scanner: test-scanner.o dynamic_buffer.o error.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

########################### DYNAMIC_BUFFER ###############################

dynamic_buffer.o: dynamic_buffer.c
	$(CC) $(CFLAGS) -c $< -o $@

test-dynamic_buffer.o: dynamic_buffer.c
	$(CC) $(CFLAGS) -DTESTING -c $< -o $@

test-dynamic_buffer: test-dynamic_buffer.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

############################### STACK ####################################

stack.o: stack.c
	$(CC) $(CFLAGS) -c $< -o $@

########################### SYMBOL_TABLE #################################

htab.o: htab.c
	$(CC) $(CFLAGS) -c $< -o $@

stable.o: stable.c htab.c stack.c scanner.c
	$(CC) $(CFLAGS) -c $< -o $@
  
############################### ERROR #####################################

error.o: error.c
	$(CC) $(CFLAGS) -c $< -o $@

########################### PARSER #################################
test-parser: parser.o scanner.o dynamic_buffer.o error.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

parser.o: parser.c scanner.c
	$(CC) $(CFLAGS) -c $< -o $@



############################### XXXXX #####################################

clean:
	rm -f $(PROG)
	rm -f *.o
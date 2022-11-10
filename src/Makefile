 ##
 # @file Makefile
 # @author Michal Ľaš (xlasmi00)
 #

CC=gcc
CFLAGS=-O2 -std=c11 -pedantic -Wall -Wextra
PROG=test-scanner test-dynamic_buffer

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

############################### ERROR #####################################

error.o: error.c
	$(CC) $(CFLAGS) -c $< -o $@

error: error.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

############################### XXXXX #####################################

clean:
	rm -f $(PROG)
	rm -f *.o
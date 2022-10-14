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

scanner.o: dynamic_buffer.c
	$(CC) $(CFLAGS) -NOT_TESTING -c $< -o $@

test-scanner.o: dynamic_buffer.c
	$(CC) $(CFLAGS) -c $< -o $@	

test-scanner: test-scanner.o dynamic_buffer.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

########################### DYNAMIC_BUFFER ###############################

dynamic_buffer.o: dynamic_buffer.c
	$(CC) $(CFLAGS) -NOT_TESTING -c $< -o $@

test-dynamic_buffer.o: dynamic_buffer.c
	$(CC) $(CFLAGS) -c $< -o $@

test-dynamic_buffer: test-dynamic_buffer.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

############################## XXXXXXX ###################################

clean:
	rm -f $(PROG)
	rm -f *.o
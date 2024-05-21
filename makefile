# Makefile for the spell checker

CC = gcc
CFLAGS = -Wall -g -std=c11 -lpthread

# Target to compile the main executable
spellchecker.c: spellchecker.c
	$(CC) $(CFLAGS) spellchecker.c -o spellchecker 

# Target to clean up generated files
clean:
	rm -f spellchecker

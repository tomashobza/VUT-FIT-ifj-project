# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -Pedantic -std=c99


# Find all source files
SRCS = $(wildcard *.c)

# Target executable name
TARGET = ifjcompiler

# Default target
all: $(TARGET)

$(TARGET): 
	#$(CC) $(CFLAGS) $(SRCS) -o $@
	$(CC) $(SRCS) -o $@

# clean, compile and run
run: clean all
	./$(TARGET)

# Clean up
clean:
	rm -f $(TARGET)

.PHONY: all clean

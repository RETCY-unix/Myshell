CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g -Iinclude
TARGET = shell
SRCDIR = src
INCDIR = include
OBJDIR = obj
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Default target
all: $(TARGET)

# Create object directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Link object files to create executable
$(TARGET): $(OBJDIR) $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Compile source files to object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCDIR)/shell.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Install shell (copy to /usr/local/bin)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET)

# Uninstall shell
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

# Release build
release: CFLAGS += -O2 -DNDEBUG
release: clean $(TARGET)

# Check for memory leaks
valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

# Static analysis
static-analysis:
	cppcheck --enable=all --inconclusive --std=c99 -I$(INCDIR) $(SRCDIR)/*.c

# Format code
format:
	indent -kr -ts4 -nut $(SRCDIR)/*.c $(INCDIR)/*.h

# Create distribution package
dist: clean
	tar -czf shell-1.0.tar.gz src/ include/ Makefile README.md

.PHONY: all clean install uninstall debug release valgrind static-analysis format dist

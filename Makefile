
# Project name
TARGET = sauve

# Folders 
OBJDIR = obj
BINDIR = bin
SRCDIR = src
INCDIR = include


# Compiler
CC = gcc
CFLAGS = -pthread -g -Wall -I $(INCDIR)

# Files
SOURCES := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(INCDIR)/*.h
OBJECTS := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) 
DEP = $(OBJECTS:%.o=%.d)
EXECUTABLE := $(BINDIR)/$(TARGET)
DOCFILE = Doxyfile
README = README.md



$(TARGET): $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@echo "\n-----------------> Linking ... "
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $@


# Dependencies to the headers are
# covered by this include
-include $(DEP) 		


$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@echo "\n-----------------> Compiling $@ ... "
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(TARGET).tar.gz : $(INCDIR) $(SRCDIR) $(DOCFILE) $(README) $(MAKEFILE_LIST)
	tar -cvzf $@ $^

.PHONY : archive clean 

archive : $(TARGET).tar.gz

clean:
	@echo "----------------- Cleaning -----------------"
	rm -f $(OBJECTS) $(EXECUTABLE) 




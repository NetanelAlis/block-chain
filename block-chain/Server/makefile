# Variables declaration
TARGET_NAME = server.out
IDIR = ../include
LDIR = ../lib
CC = g++
CFLAGS = -I$(IDIR) -g
LIBS = -lz

# ? means that value will be set only if ODIR was not set for example from command line: make ODIR=native
ODIR ?= build

# SRC will hold an array of all the .cpp files
SRCS := $(subst ./,,$(shell find . -maxdepth 1 -name "*.cpp"))

# OBJS will hold an array of the corresponding .o to the .cpp files
OBJS := $(addprefix $(ODIR)/,$(patsubst %.cpp,%.o,$(SRCS)))

# Pattern rule for object files
$(ODIR)/%.o: %.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

# Rule for creating the final executable
$(ODIR)/$(TARGET_NAME): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

# Phony target for cleaning up build artifacts
.PHONY: clean
clean:
	rm -rf $(ODIR)/*.o $(ODIR)/$(TARGET_NAME)

# Include dependencies
-include $(OBJS:.o=.d)

# Rule to generate a file of dependencies (uses the -MM GCC flag)
$(ODIR)/%.d: %.cpp
	$(CC) $(CFLAGS) -MM -MT '$(ODIR)/$*.o' $< -MF $@

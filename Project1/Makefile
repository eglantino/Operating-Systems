# Compiler and compiler flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -g

# Source files
SRCS = linked_hash.cpp reverse_list.cpp voters.cpp mvote.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
EXEC = mvote

# Default target
all: $(EXEC)

# Rule to build the executable
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to build object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean target to remove object files and the executable
clean:
	rm -f $(OBJS) $(EXEC)

.PHONY: all clean

CC = g++
CFLAGS = -Wall -Wextra -pedantic

# The final executable program name
TARGET = cmpcat

# Object files
OBJ = cmpcats.o Compare.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile the cmpcats.cpp to an object file
cmpcats.o: cmpcats.cpp
	$(CC) $(CFLAGS) -c cmpcats.cpp

# Compile the compare.cpp to an object file
Compare.o: Compare.cpp
	$(CC) $(CFLAGS) -c Compare.cpp

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean

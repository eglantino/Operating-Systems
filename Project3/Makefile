CC = g++
CFLAGS = -Wall -Wextra -pedantic
LDFLAGS = -lrt -lpthread
all: main reader writer

main: Init.cpp
	$(CC) $(CFLAGS) -o main Init.cpp $(LDFLAGS)

reader: reader.cpp
	$(CC) $(CFLAGS) -o reader reader.cpp $(LDFLAGS)

writer: writer.cpp
	$(CC) $(CFLAGS) -o writer writer.cpp $(LDFLAGS)

clean:
	rm -f main reader writer

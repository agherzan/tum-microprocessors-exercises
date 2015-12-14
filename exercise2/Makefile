CC = gcc
CFLAGS  = -O3

TARGET = cache-line cache-size

all: $(TARGET)

cache-line: cache-line.c
	$(CC) $(CFLAGS) -o cache-line cache-line.c

cache-size: cache-size.c
	$(CC) $(CFLAGS) -o cache-size cache-size.c

clean:
	$(RM) $(TARGET)

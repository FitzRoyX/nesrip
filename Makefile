CC = gcc
CFLAGS = -Iinclude
SRCS = $(wildcard src/*.c src/sha_2/sha-256.c)
OBJS = $(SRCS:.c=.o)
TARGET = nesrip.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
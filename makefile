CC = gcc
CFLAGS = -Wall -Wextra -Isrc
LDFLAGS = -lpthread

SRC = src/main.c src/buffer.c src/producer.c src/consumer.c src/config.c src/common/utils.c

TARGET = main

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
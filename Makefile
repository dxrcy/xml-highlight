CC = gcc
CFLAGS = -Wall -Wextra -pedantic

TARGET = xmlparse

main:
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c


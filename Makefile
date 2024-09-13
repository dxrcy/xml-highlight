CC = gcc
CFLAGS = -Wall -Wextra -pedantic

TARGET = xml-highlight
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

install:
	install -d $(BINDIR)
	install $(TARGET) $(BINDIR)

uninstall:
	rm -f $(BINDIR)/$(TARGET)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	cat example.xml | ./$(TARGET)

.PHONY: install uninstall clean run


CC      = gcc
# -Wall -Wextra: Show all warnings
# -O2: Basic optimization
CFLAGS  = -Wall -Wextra -O2
LIBS    = -lcurl
TARGET  = httpx
PREFIX  = /usr/local

all: $(TARGET)

$(TARGET): httpx.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	install -D -m 755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)

.PHONY: all clean install uninstall

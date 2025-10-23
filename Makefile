CC = gcc
CFLAGS = -lcurl

all: httpx

httpx: httpx.c
	$(CC) -o $@ $< $(CFLAGS)

clean:
	rm -f httpx

install: httpx
	sudo cp httpx /usr/local/bin/

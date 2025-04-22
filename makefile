.PHONY: all

all:
	cc -o sponge sponge.c

.PHONY: install

install:
	mkdir -p /usr/local/bin
	mkdir -p /usr/local/share/man/man1
	cc -o sponge sponge.c
	cp sponge /usr/local/bin
	cp sponge.1 /usr/local/share/man/man1

.PHONY: test

test:
	cc -o sponge sponge.c && ./test.sh

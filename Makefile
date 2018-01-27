CC ?= gcc -std=c99
CFLAGS ?= -O3
BASE := $(shell pwd)
DEBUG ?= -DDEBUG
LIBS := libcrypto.a
CFLAGS += $(DEBUG)
all: rcoind

include crypto/crypto.mk

RCOIN_FILES := $(wildcard src/*.c)

rcoind: $(RCOIN_FILES:.c=.o) $(LIBS) Makefile
	$(CC) $(LDFLAGS) -o rcoind $(RCOIN_FILES:.c=.o) $(LIBS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
clean:
	rm -f src/*.o

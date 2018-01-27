CC ?= gcc -std=c99
CFLAGS ?= -O3
BASE := $(shell pwd)
DEBUG ?= -DDEBUG -g
LIBS := libcrypto.a
CFLAGS += $(DEBUG)
SYSLIBS =
ifeq ($(WINDIR),)
	SYSLIBS = -lpthread
endif
all: rcoind

include crypto/crypto.mk

RCOIN_FILES := $(wildcard src/*.c)

rcoind: $(RCOIN_FILES:.c=.o) $(LIBS) Makefile
	$(CC) $(LDFLAGS) -o rcoind $(RCOIN_FILES:.c=.o) $(LIBS) $(SYSLIBS)

src/%.o: src/%.c 
	$(CC) $(CFLAGS) -c -o $@ $<
clean:
	rm -f src/*.o

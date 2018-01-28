CC ?= gcc -std=c99
CFLAGS ?= -O3
BASE := $(shell pwd)
DEBUG ?= -DDEBUG -g
LIBS := libcrypto.a
CFLAGS += $(DEBUG)
SYSLIBS =
ifeq ($(WINDOWS),1)
	WINDIR = /c/mingw
endif
ifeq ($(WINDIR),)
	SYSLIBS = -lpthread -ldl
else
	SYS_FILES = src/win32/mman.c
	CFLAGS += -Isrc/win32
endif
all: rcoind

include crypto/crypto.mk

RCOIN_FILES := $(wildcard src/*.c) $(SYS_FILES)

rcoind: $(RCOIN_FILES:.c=.o) $(LIBS) Makefile
	$(CC) $(LDFLAGS) -o rcoind $(RCOIN_FILES:.c=.o) $(LIBS) $(SYSLIBS)

src/%.o: src/%.c 
	$(CC) $(CFLAGS) -c -o $@ $<
clean:
	rm -f src/*.o

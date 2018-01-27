CRYPTO_FILES := $(wildcard $(BASE)/crypto/edwards/*.c) $(wildcard $(BASE)/crypto/sha512/*.c $(BASE)/crypto/aes/*.c)
CFLAGS += -I$(BASE)/crypto/edwards -I$(BASE)/crypto/sha512 -I$(BASE)/crypto/aes
libcrypto.a: $(CRYPTO_FILES)
	cd crypto; $(CC) $(CFLAGS) -c $(CRYPTO_FILES)
	$(AR) rc libcrypto.a crypto/*.o

libcrypto.clean:
	rm -f crypto/*.o

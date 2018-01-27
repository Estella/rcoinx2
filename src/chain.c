#include <unistd.h>
#include "main.h"
#include "block.h"
#include <string.h>
#include "debug.h"
#include <errno.h>
#ifndef WIN32
	#include <sys/stat.h>
#endif
static uint64_t _mainchain_length;
static uint64_t _altchain_length;
void write_block(int is_main_chain, uint64_t id, struct block *block);
FUNCTION void read_block(int is_main_chain, uint64_t id, struct block *block) {
	char path[256];
	snprintf(path, 256, "%s/blockchain/%s/%d.dat", get_options()->datadir, is_main_chain ? "main" : "alt", id);
	FILE *f = fopen(path, "rb");
	if (!f) {
		perror("blockchain");
		fatal_error("Can't read block from chain!");
	}
	fread(block, 1, sizeof(struct block), f);
	fclose(f);
}
static void update_blockchain_length(int is_main_chain, uint64_t length) {
	char path[256];
	snprintf(path, 256, "%s/blockchain/%s/height.txt", get_options()->datadir, is_main_chain ? "main" : "alt");
	FILE *f = fopen(path, "w");
	if (!f) {
		perror("blockchain");
		fatal_error("Can't write blockchain length!");
	}
	fprintf(f, "%lld\n", length);
	fclose(f);
}
static uint64_t get_blockchain_length(int is_main_chain) {
	char path[256];
	snprintf(path, 256, "%s/blockchain/%s/height.txt", get_options()->datadir, is_main_chain ? "main" : "alt");
	FILE *f = fopen(path, "w");
	if (!f) {
		return 0;
	}
	uint64_t length = 0;
	fscanf(f, "%lld\n", length);
	fclose(f);
	return length;
}
FUNCTION void init_blockchain() {
	char path[256];
	snprintf(path, 256, "%s/%s", get_options()->datadir, "blockchain");
	IF_DEBUG(printf("Blockchain path: %s\n", path));
	if (mkdir(path, 0755) != 0);
		if (errno != EEXIST && errno > 0) {
			perror("blockchain");
			fatal_error("Failed");
		}
	char path2[256];
	snprintf(path2, 256, "%s/main", path);
	IF_DEBUG(printf("Main chain path: %s\n", path2));
	mkdir(path2, 0755);
	snprintf(path2, 256, "%s/alt", path);
	mkdir(path2, 0755);
	_mainchain_length = get_blockchain_length(1);
	_altchain_length = get_blockchain_length(0);
	write_block(1, 0, &genesis_block);
	if (_mainchain_length == 0) {
		_mainchain_length = 1;
		update_blockchain_length(1, _mainchain_length);
	}
	printf("Blockchain heights: main = %lld, alt = %lld\n", _mainchain_length, _altchain_length);
}
FUNCTION void write_block(int is_main_chain, uint64_t id, struct block *block) {
	char path[256];
	snprintf(path, 256, "%s/%s/%s/%d.dat", get_options()->datadir, "blockchain", is_main_chain ? "main" : "alt", id);
	FILE *f = fopen(path, "wb");
	if (!f) {
		perror("blockchain");
		fatal_error("Can't write to blockchain database");
	}
	fwrite(block, 1, sizeof(struct block), f);
	fclose(f);
}

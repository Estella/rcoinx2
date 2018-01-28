#define _CHAIN
#include "log.h"
#include <time.h>
#include <unistd.h>
#include "main.h"
#include "block.h"
#include <string.h>
#include "debug.h"
#include <errno.h>
#include "base32.h"
#include "sqlite3.h"
#ifndef WIN32
	#include <sys/stat.h>
#endif
static sqlite3* db;
static uint64_t _mainchain_length;
static uint64_t __tx_search_base;
static uint64_t _altchain_length;
uint64_t get_height() {
	return _mainchain_length;
}
uint64_t __get_alt_height() {
	return _altchain_length;
}
int _internal_rdb(void* ptr, int argc, char **argv, char **colnames) {
	int len;
	static char compbuf[256000];
	if (argc != 1)
		log_fatal("Failed to read block from database.");
	len = base32_decode(argv[0], compbuf, 256000);
	block_decompress(compbuf, len, ptr);
	return 0;
}
void write_block(int is_main_chain, uint64_t id, struct block *block);
FUNCTION void read_block(int is_main_chain, uint64_t id, struct block *block) {
	char statement[256];
	sprintf(statement, "SELECT data FROM blocks WHERE id = %lld;", id);
	static int locked = 0;
	while (locked);
	locked = 1;
	if(sqlite3_exec(db, statement, _internal_rdb, block, NULL)) log_fatal("Failed to read block %d.", id);
	locked = 0;
}
int transaction_spent(uint8_t* signature) {
	char sigbuf[128];
	base32_encode(signature, 64, sigbuf, 128);
	/*for (int i = __tx_search_base; i < get_height(); i++) {
		struct block b;
		read_block(1, i, &b);
		for (int t = 0; t < b.num_tx; t++) {
			if (!memcmp(b.transactions[t].signature, signature, 64)) {
				map_set(transaction_cache, sigbuf, 1);
			}
		}
	}
	return map_has(transaction_cache, sigbuf);*/
	return 0;
}
uint64_t get_balance_for_address(uint8_t* public) {
	struct block b; char basebuf[64];
	base32_encode(public, 32, basebuf, 64);
	if (map_has(balance_cache, basebuf)) return map_get(balance_cache, basebuf);
	uint64_t ret = 0;
	for (int i = 0; i < get_height(); i++) {
		read_block(1, i, &b);
		for (int t = 0; t < b.num_tx; t++) {
			if (!memcmp(b.transactions[t].body.to, public, 32)) {
				ret += b.transactions[t].body.amount;
			}
			if (!memcmp(b.transactions[t].body.from, public, 32)) {
				ret -= b.transactions[t].body.amount;
			}
		}
	}
	map_set(balance_cache, basebuf, ret);
	return ret;
}
static void update_blockchain_difficulty(int is_main_chain, uint32_t difficulty) {
	char statement[256];
	sprintf(statement, "insert or replace into misc (key, valueint) values('difficulty%d', %ld);", is_main_chain, difficulty);
	if (sqlite3_exec(db, statement, NULL, NULL, NULL)) log_fatal("Failed to update blockchain difficulty");
}
int _internal_gbdf(void* ptr, int argc, char **argv, char **colnames) {
	if (argc != 1)
		log_fatal("Failed to query blockchain difficulty");
	*(uint32_t*)ptr = atol(argv[0]);
	return 0;
}
static uint32_t get_blockchain_difficulty(int is_main_chain) {
	uint32_t difficulty; char statement[256];
	sprintf(statement, "select valueint from misc where key = 'difficulty%d';", is_main_chain);
	sqlite3_exec(db, statement, _internal_gbdf, &difficulty, NULL);
	return difficulty;
}
int _internal_gbcl(void* lengthptr, int argc, char **argv, char **colnames) {
	IF_DEBUG(puts(argv[0]));
	if (argc != 1)
		log_fatal("Failed to query blockchain length");
	*(uint64_t*)lengthptr = atoll(argv[0]);
	return 0;
}
static uint64_t get_blockchain_length(int is_main_chain) {
	uint64_t length; char statement[256];
	sprintf(statement, "select id from blocks where mainchain = %d order by id desc limit 1;", is_main_chain);
	sqlite3_exec(db, statement, _internal_gbcl, &length, NULL);
	return length + 1;
}
void setup_blockchain() {
	sqlite3_exec(db, "CREATE TABLE blocks (id int, data string, hash string, lasthash string, mainchain int);", NULL, NULL, NULL);
	sqlite3_exec(db, "CREATE TABLE balances (balance int, address string);", NULL, NULL, NULL);
	sqlite3_exec(db, "CREATE TABLE txcache (signature string);", NULL, NULL, NULL);
	sqlite3_exec(db, "CREATE TABLE misc (key string, valueint int, valuestr string);", NULL, NULL, NULL);
	write_block(1, 0, &genesis_block);
}
FUNCTION void init_blockchain() {
	char path[256]; int first_time = 0;
	memcpy(genesis_block.transactions[0].body.to, __GENESIS_BLOCK__(), 32);
	snprintf(path, 256, "%s/%s", get_options()->datadir, "blockchain.db");
	IF_DEBUG(printf("Blockchain path: %s\n", path));
	if (sqlite3_open_v2(path, &db, SQLITE_OPEN_READWRITE, NULL))
		if (sqlite3_open_v2(path, &db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL))
			log_fatal("Fatal error: can't open blockchain\n");
		else
			first_time = 1;
	if (first_time) setup_blockchain();
	_mainchain_length = get_blockchain_length(1);
	_altchain_length = get_blockchain_length(0);
	last_difficulty = get_blockchain_difficulty(1);
	last_alt_difficulty = get_blockchain_difficulty(0);
	printf("Blockchain heights: main = %lld, alt = %lld\n", _mainchain_length, _altchain_length);
}
FUNCTION void write_block(int is_main_chain, uint64_t id, struct block *block) {
	static int locked = 0;
	static char blockbuf[256000];
	static char compblock[70000];
	int osz;
	block_compress(block, compblock, osz);
	while (locked);
	locked = 1;
	int pos = sprintf(blockbuf, "insert or replace into blocks(id, mainchain, data, hash, lasthash) values(%lld, %d, '", id, is_main_chain);
	pos += base32_encode(compblock, osz, blockbuf + pos, 256000 - pos);
	pos += sprintf(blockbuf + pos, "', '");
	pos += base32_encode(block->hash, 64, blockbuf + pos, 256000 - pos);
	pos += sprintf(blockbuf + pos, "', '");
	pos += base32_encode(block->lasthash, 64, blockbuf + pos, 256000 - pos);
	pos += sprintf(blockbuf + pos, "');");
	if (sqlite3_exec(db, blockbuf, NULL, NULL, NULL)) log_fatal("Failed to write block %lld to database.", id);
	locked = 0;
}

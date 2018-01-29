#define _CHAIN
#include "tinycthread.h"
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
int _internal_gbcl(void* lengthptr, int argc, char **argv, char **colnames) {
	if (argc != 1)
		log_fatal("Failed to query blockchain length");
	*(uint64_t*)lengthptr = atoll(argv[0]);
	return 0;
}
static uint64_t get_blockchain_length(int is_main_chain) {
	uint64_t length = (uint64_t)-1; char statement[256];
	sprintf(statement, "select id from blocks where mainchain = %d order by id desc limit 1;", is_main_chain);
	log_assert(0 == sqlite3_exec(db, statement, _internal_gbcl, &length, NULL), sqlite3_errmsg(db));
	log_assert(length != (uint64_t)-1 || is_main_chain != 1, "Can't get height of main blockchain!");
	if (length == (uint64_t)-1) length = 0;
	return length + 1;
}
int _internal_gbcb(void* lengthptr, int argc, char **argv, char **colnames) {
	if (argc != 1)
		log_fatal("Failed to query blockchain length");
	*(uint64_t*)lengthptr = atoll(argv[0]);
	return 0;
}
static uint64_t get_blockchain_base(int is_main_chain) {
	uint64_t length; char statement[256];
	sprintf(statement, "select id from blocks where mainchain = %d order by id asc limit 1;", is_main_chain);
	sqlite3_exec(db, statement, _internal_gbcb, &length, NULL);
	return length;
}
uint64_t get_height() {
	_mainchain_length = get_blockchain_length(1);
	_altchain_length = get_blockchain_length(0);
	return _mainchain_length;
}
uint64_t __get_alt_height() {
	_mainchain_length = get_blockchain_length(1);
	_altchain_length = get_blockchain_length(0);
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
FUNCTION void read_block(int is_main_chain, uint64_t id, struct block *block) {
	char statement[256];
	sprintf(statement, "SELECT data FROM blocks WHERE id = %lld;", id);
	static int locked = 0;
	while (locked);
	locked = 1;
	if(sqlite3_exec(db, statement, _internal_rdb, block, NULL)) log_fatal("Failed to read block %d.", id);
	locked = 0;
	IF_DEBUG(print_block(block));
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
struct __internal_state_gbfa {
	char *myaddr;
	uint64_t* ret;
};
int _internal_gbfa(void *ptr, int argc, char **argv, char **colnames) {
	struct __internal_state_gbfa* state = ptr;
	if (argc != 3) log_fatal("Assertion failure " __FILE__);
	IF_DEBUG(printf("%s %s\n", argv[1], argv[2]));
	if (!strcmp(argv[1], state->myaddr))
		*state->ret -= atoll(argv[0]);
	else
		*state->ret += atoll(argv[0]);
	return 0;
}
uint64_t get_balance_for_address(uint8_t* public) {
	struct block b; char basebuf[64]; uint64_t ret = 0;
	base32_encode(public, 32, basebuf, 64);
	IF_DEBUG(printf("getbalance(%s) -> ?\n", basebuf));
	char statement[384];
	struct __internal_state_gbfa state = {basebuf, &ret};
	sprintf(statement, "SELECT amount, xfrom, xto FROM txcache WHERE xfrom = '%s' or xto = '%s';", basebuf, basebuf);
	if(sqlite3_exec(db, statement, _internal_gbfa, &state, NULL)) { IF_DEBUG(puts(sqlite3_errmsg(db))); }
	return ret;
}
void update_blockchain_difficulty(int is_main_chain, uint32_t difficulty) {
	char statement[256];
	sprintf(statement, "delete from misc where key = 'difficulty%d'; insert or replace into misc (key, valueint) values('difficulty%d', %ld);", is_main_chain, is_main_chain, difficulty);
	if (sqlite3_exec(db, statement, NULL, NULL, NULL)) log_fatal("Failed to update blockchain difficulty");
}
int _internal_gbdf(void* ptr, int argc, char **argv, char **colnames) {
	if (argc != 1)
		log_fatal("Failed to query blockchain difficulty");
	*(uint32_t*)ptr = atol(argv[0]);
	return 0;
}
static uint32_t get_blockchain_difficulty(int is_main_chain) {
	uint32_t difficulty = 1; char statement[256];
	sprintf(statement, "select valueint from misc where key = 'difficulty%d';", is_main_chain);
	sqlite3_exec(db, statement, _internal_gbdf, &difficulty, NULL);
	return difficulty;
}
#define A(a) log_assert(0 == a, sqlite3_errmsg(db));
void setup_blockchain() {
	A(sqlite3_exec(db, "CREATE TABLE blocks (id int, data string, hash string, lasthash string, time int, mainchain int);", NULL, NULL, NULL));
	A(sqlite3_exec(db, "CREATE TABLE balances (balance int, address string);", NULL, NULL, NULL));
	A(sqlite3_exec(db, "CREATE TABLE txcache (blockid int, mainchain int, signature string, xfrom string, xto string, amount int);", NULL, NULL, NULL));
	A(sqlite3_exec(db, "CREATE TABLE misc (key string, valueint int, valuestr string);", NULL, NULL, NULL));
}
#undef A
static struct _internal_chains {
	int all_chains[512];
	uint64_t lastblocktime[512];
	int is_orphan_block[512];
	int num_chains;
} _internal_chains;
int _internal_cleanup_isorphan(void* result, int argc, char **argv, char **colnames) {
	log_assert(argc == 1, "What!?");
	*((int*)result) = 1;
	return 0;
}
int _internal_new_chain_id(void *result, int argc, char **argv, char **colnames) {
	log_assert(argc == 1, "What!?");
	*((int*)result) = atoi(argv[0]) + 1;
	return 0;
}
int new_chain_id() {
	int result = 0;
	log_assert(0 == sqlite3_exec(db, "SELECT max(mainchain) FROM blocks;", _internal_new_chain_id, &result, NULL), "Failed");
	return result;
}
int chain_is_orphan(int chain_id) {
	int result = 0; char statement[1024];
	sprintf(statement, "SELECT id FROM blocks WHERE EXISTS(SELECT * FROM blocks WHERE mainchain = %d AND id = %lld);", chain_id, get_blockchain_base(chain_id) + 1);
	log_assert(0 == sqlite3_exec(db, statement, _internal_cleanup_isorphan, &result, NULL), "Failed");
	return result;
}
int _internal_cleanup_getchains(void* UNUSED, int argc, char **argv, char **colnames) {
	log_assert(argc == 2, "What!?");
	_internal_chains.all_chains[_internal_chains.num_chains++] = atoi(argv[0]);
	_internal_chains.lastblocktime[_internal_chains.num_chains-1] = atoll(argv[1]);
	return 0;
}
void periodic_cleanup() {
	// Delete orphan blocks older than 2 minutes and delete alternate chains older than 10 minutes
	_internal_chains.num_chains = 0;
	log_assert(0 == sqlite3_exec(db, "SELECT DISTINCT mainchain, time FROM blocks WHERE mainchain <> 1;", _internal_cleanup_getchains, NULL, NULL), sqlite3_errmsg(db));
	for (int i = 0; i < _internal_chains.num_chains; i++) {
		int chainid = _internal_chains.all_chains[i];
		int isorphan = chain_is_orphan(_internal_chains.all_chains[i]);
		uint64_t lasttime = _internal_chains.lastblocktime[i];
		char statement[256];
		if (((time(NULL) - lasttime) > 120 && isorphan) || ((time(NULL) - lasttime) > 600)) {
			sprintf(statement, "DELETE FROM blocks WHERE mainchain = %d;", chainid);
			log_assert(0 == sqlite3_exec(db, statement, NULL, NULL, NULL), "Failed to purge forked blockchain");
		}
	}
}
int cleanup_thread(void* UNUSED) {
	log_info("Started blockchain cleanup thread...");
	while (1) {
		periodic_cleanup();
		thrd_sleep(&(struct timespec){.tv_sec=120}, NULL);
	}
}
void switch_chains(int new_chain) {
	log_assert(new_chain != 1, "Can't switch from main chain to main chain!");
	sqlite3_mutex_enter(sqlite3_db_mutex(db));
	char statement[1024];
	sprintf(statement, "BEGIN; DELETE FROM blocks WHERE mainchain = 1 AND id >= %lld; UPDATE blocks SET mainchain = 1 WHERE mainchain = %d; END;", get_blockchain_base(new_chain), new_chain);
	if(sqlite3_exec(db, statement, NULL, NULL, NULL))
		log_fatal("Failed to switch blockchains (alternate chain %id to main chain).", new_chain);
	sqlite3_mutex_leave(sqlite3_db_mutex(db));
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
	write_block(1, 0, &genesis_block);
	_mainchain_length = get_blockchain_length(1);
	_altchain_length = get_blockchain_length(0);
	last_difficulty = get_blockchain_difficulty(1);
	last_alt_difficulty = get_blockchain_difficulty(0);
	thrd_t t;
	thrd_create(&t, cleanup_thread, NULL);
}
FUNCTION void write_block(int is_main_chain, uint64_t id, struct block *block) {
	static int locked = 0;
	static char blockbuf[256000];
	static char compblock[70000];
	int osz = 70000; int failed;
	if (failed = block_compress(block, compblock, osz)) log_fatal("Failed to compress block: err %d.\n", failed);
	while (locked);
	locked = 1;
	sqlite3_exec(db, "CREATE UNIQUE INDEX idx_blockhash ON blocks (hash);", NULL, NULL, NULL);
	IF_DEBUG(printf("IsMainChain -> %d\n", is_main_chain));
	int pos = sprintf(blockbuf, "insert or replace into blocks(id, time, mainchain, data, hash, lasthash) values(%lld, %lld, %d, '", id, block->timestamp, is_main_chain);
	IF_DEBUG(printf("Statement -> %s...);\n", blockbuf));
	pos += base32_encode(compblock, osz, blockbuf + pos, 256000 - pos);
	pos += sprintf(blockbuf + pos, "', '");
	pos += base32_encode(block->hash, 64, blockbuf + pos, 256000 - pos);
	pos += sprintf(blockbuf + pos, "', '");
	pos += base32_encode(block->lasthash, 64, blockbuf + pos, 256000 - pos);
	pos += sprintf(blockbuf + pos, "');");
	//sqlite3_exec(db, "CREATE TABLE txcache (blockid int, mainchain int, signature string, xfrom string, xto string, amount int);", NULL, NULL, NULL);
	if (sqlite3_exec(db, blockbuf, NULL, NULL, NULL)) log_fatal("Failed to write block %lld to database: %s.", id, sqlite3_errmsg(db));
	locked = 0;
	char statement[1536];
	sqlite3_exec(db, "CREATE UNIQUE INDEX idx_txsig ON txcache (signature);", NULL, NULL, NULL);
	for (int i = 0; i < block->num_tx; i++) {
		int l = sprintf(statement, "insert or replace into txcache(mainchain, blockid, signature, xfrom, xto, amount) values(%d, %lld, '", is_main_chain, id);
		l += base32_encode(block->transactions[i].signature, 64, statement + l, 1536);
		l += sprintf(statement + l, "', '");
		l += base32_encode(block->transactions[i].body.from, 32, statement +l, 1536);
		l += sprintf(statement + l, "', '");
		l += base32_encode(block->transactions[i].body.to, 32, statement + l, 1536);
		l += sprintf(statement + l, "', %lld);", block->transactions[i].body.amount);
		if (sqlite3_exec(db, statement, NULL, NULL, NULL)) log_fatal("Failed to write transaction cache: %s.", sqlite3_errmsg(db));
	}
}

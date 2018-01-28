#include <stdio.h>
#include <string.h>
#include "rpc.h"
#include "block.h"
#include "chain.h"
#include "pow.h"
#include "main.h"
#include "tinycthread.h"
#include "debug.h"
static char datadir[256];
static thrd_t rpcserver;
struct map_t* balance_cache;
struct cache_entry {
	char address[64];
	uint64_t value;
};
static struct options my_opts = {
	.port = 12991,
	.allow_all = 0
};
char *defaultopt(char* a, char *b) {
	char *c = getenv(a);
	if (!c) c = b;
	return c;
}
struct options* get_options() {
	return &my_opts;
}
int main(int argc, char **argv) {
	balance_cache = new_map();
	if (argc == 2 && (argv[1][1] == 'h' || argv[1][2] == 'h')) {
		printf("RCOINX/2.0 Block size = %d, transaction size = %d\n", sizeof(struct block), sizeof(struct tx));
		printf("Usage: rcoind [--option=value]...\n");
		return 1;
	}
	for (int i = 1; i < argc; i++) {
		if (strlen(argv[i]) < 4) {
			printf("Bad argument: %s\n", argv[i]);
			return 1;
		}
		putenv(argv[i] + 2);
	}
	snprintf(datadir, 256, "%s/%s/%s", USERDATADIR, APPDATADIR, "rcoinx");
	my_opts.datadir = defaultopt("datadir", datadir);
	my_opts.port = atoi(defaultopt("port", "12991"));
	mkdir(datadir, 0755);
	printf("loading blockchain...\n");
	init_blockchain();
	printf("loaded blockchain...\n");
	printf("loading rpc server...\n");
	thrd_create(&rpcserver, init_rpc_server, (void*)my_opts.port);
	printf("loaded rpc server..\n");
	while (1) {
		thrd_yield();
	}
}

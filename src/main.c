#include <stdio.h>
#include <string.h>
#include "block.h"
#include "chain.h"
#include "pow.h"
#include "main.h"
static char datadir[256];
static struct options my_opts;
struct options* get_options() {
	return &my_opts;
}
int main() {
	snprintf(datadir, 256, "%s/%s/%s", USERDATADIR, APPDATADIR, "rcoinx");
	my_opts.datadir = datadir;
	mkdir(datadir, 0755);
	init_blockchain();
}

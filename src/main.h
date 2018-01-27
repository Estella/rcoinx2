#pragma once
#include <stdio.h>
#include <stdint.h>
struct options {
	char *datadir;
	int allow_all;
	char *password;
	int port;
};

struct options* get_options();
#ifdef WIN32
	#define USERDATADIR getenv("USERPROFILE")
	#define APPDATADIR getenv("APPDATA")
#else
	#define USERDATADIR getenv("HOME")
	#define APPDATADIR ""
	#include <sys/stat.h>
#endif

#include <stdlib.h>
#ifndef _CHAIN
extern
#endif
uint32_t last_difficulty, last_alt_difficulty;
#define fatal_error(s) (fprintf(stderr, "%s\n", s), abort(), 0);
#include "block.h"
#include "chain.h"
#include "pow.h"
static inline uint32_t get_difficulty(int mainchain) {
	if (get_height() < 3) return calc_target(0,0,0);
	struct block a, b;
	uint64_t height = mainchain ? get_height() : __get_alt_height();
	read_block(mainchain, height - 1, &a);
	read_block(mainchain, height - 2, &b);
	return calc_target(mainchain ? last_difficulty : last_alt_difficulty, a.timestamp, b.timestamp);
}

#pragma once
#include <stdio.h>
struct options {
	char *datadir;
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
#define fatal_error(s) (fprintf(stderr, "%s\n", s), abort(), 0);

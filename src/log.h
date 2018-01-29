#pragma once
#include <time.h>
#include <string.h>
#include <stdio.h>
static inline char* _tstamp() {
	static char myts[64];
	time_t a;
	time(&a);
	strcpy(myts, ctime(&a));
	myts[strlen(myts)-1] = 0;
	return myts;
}
#define log_fatal(...) (fprintf(stderr, "FATAL [%s] ", _tstamp()), fprintf(stderr, __VA_ARGS__), abort(), 1)
#define log_warn(...) (fprintf(stderr, "WARNING [%s] ", _tstamp()), fprintf(stderr, __VA_ARGS__), fprintf(stderr, "\n"))
#define log_info(...) (fprintf(stderr, "INFO [%s] ", _tstamp()), fprintf(stderr, __VA_ARGS__), fprintf(stderr, "\n"))
#define log_assert(a, b) if (!(a)) log_fatal("Assertion failure: %s on %s:%d (Comment: %s)", #a, __FILE__, __LINE__, b)

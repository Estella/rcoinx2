#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
enum { JSON_END = 0, JSON_OBJ, JSON_INT, JSON_INT64, JSON_UINT, JSON_UINT64, JSON_FLOAT, JSON_STR };
static const char* fmtstrs[] = { "", "%s", "%d", "%lld", "%u", "%llu", "%.6f", "\"%s\"" };
static int easy_json_encode(char *out, int outlen, ...) {
	va_list va;
	va_start(va, outlen);
	int target = 0;
	target += snprintf(out, outlen - target, "{");
	while (1) {
		int type = va_arg(va, int);
		if (!type || type > JSON_STR) break;
		char *name = va_arg(va, char*);
		union {
			int iv;
			int64_t i64v;
			char *sv;
			double dv;
		} value;
		if (type == JSON_INT || type == JSON_UINT) {
			value.iv = va_arg(va, int);
		} else if (type == JSON_INT64 || type == JSON_UINT64) {
			value.i64v = va_arg(va, int64_t);
		} else if (type == JSON_FLOAT) {
			value.dv = va_arg(va, double);
		} else if (type == JSON_OBJ || type == JSON_STR) {
			value.sv = va_arg(va, char*);
		} else {
			abort();
		}
		const char *fmtstr = fmtstrs[type];
		target += snprintf(out + target, outlen - target, "\n\"%s\": ", name);
		if (type == JSON_FLOAT)
		target += snprintf(out + target, outlen - target, fmtstr, value.dv);
		else if (type == JSON_STR || type == JSON_OBJ)
		target += snprintf(out + target, outlen - target, fmtstr, value.sv);
		else
		target += snprintf(out + target, outlen - target, fmtstr, (type == JSON_INT || type == JSON_UINT) ? value.iv :
			(type == JSON_UINT64 || type == JSON_INT64) ? value.i64v : 0); // I have to do this because gcc is stupid
		target += snprintf(out + target, outlen - target, ",");
	}
	va_end(va);
	*(out + target - 1) = '\0';
	snprintf(out + target - 1, outlen - target + 1, "\n}");
}

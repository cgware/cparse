#ifndef LOC_H
#define LOC_H

#include <stddef.h>

typedef struct loc_s {
	size_t off;
	size_t len;
} loc_t;

#endif

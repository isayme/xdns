#ifndef _LIBBM_H
#define _LIBBM_H

#include "defs.h"

#define XSIZE	256
#define ASIZE	256

#define BM_VERSION	1

int BM(unsigned char *pattern, int pattern_len, unsigned char *text, int text_len);

#endif

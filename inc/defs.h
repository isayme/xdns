#ifndef _DEFS_H
#define _DEFS_H

// define function return value macro
#define R_ERROR		-1
#define R_OK		0

// typedef some data type for global use
typedef signed char	        INT8;
typedef unsigned char       UINT8;

typedef signed short        INT16;
typedef unsigned short      UINT16;

typedef signed int          INT32;
typedef unsigned int        UINT32;

typedef signed long long    INT64;
typedef unsigned long long  UINT64;


#define  MAX(a,b)	(((a)>(b))?(a):(b))
#define  MIN(a,b)	(((a)<(b))?(a):(b))

#endif

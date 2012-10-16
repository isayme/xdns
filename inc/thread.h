#ifndef _THREAD_H
#define _THREAD_H

#include <pthread.h>
#include "defs.h"

typedef pthread_mutex_t CS_T;

INT32 CS_INIT(CS_T *cs);

INT32 CS_ENTER(CS_T *cs);

INT32 CS_LEAVE(CS_T *cs);

INT32 CS_DEL(CS_T *cs);

#endif
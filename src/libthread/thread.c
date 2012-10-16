#include "thread.h"

INT32 CS_INIT(CS_T *cs)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
	return pthread_mutex_init(cs,&attr);
}

INT32 CS_ENTER(CS_T *cs)
{
	return pthread_mutex_lock(cs);
}

INT32 CS_LEAVE(CS_T *cs)
{
	return pthread_mutex_unlock(cs);
}

INT32 CS_DEL(CS_T *cs)
{
	return pthread_mutex_destroy(cs);
}
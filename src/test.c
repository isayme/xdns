#include <stdio.h>
#include <unistd.h>

#include "liblog.h"
#include "libthreadpool.h"

void *task(void *arg)
{
	PRINTF(LEVEL_INFORM, "task print : [%s].\n", (char*)arg);

	return 0;
}

char str[10][15] = {
"aa",
"bbbbbb",
"ccccc",
"ddd",
"eeee",
"fffffff",
"ggggs"
};

int main()
{
	thread_pool_t *tp;
	int i;

	tp = tp_create(5);
    if (NULL != tp)
    {
        PRINTF(LEVEL_INFORM, "tp create ok.\n");
    }
	for (i = 0; i < 10; i++)
	{
		tp_add_task(tp, task, str[i%7]);
	}

	sleep(3);
	tp_destroy(tp);
	return 0;
}

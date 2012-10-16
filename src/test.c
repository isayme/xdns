#include <stdio.h>
#include "defs.h"
#include "log.h"
#include "thread.h"
#include "daemon.h"
#include "bm.h"

int main(int argc, char **argv)
{
	printf("makefile test.\n");
    
    PRINTF(LEVEL_ERROR, "bm test : asd in jhgfasdklj pos = %d\n", BM("asd", 3, "jhgfasdklj", 10));
    
    PRINTF(LEVEL_ERROR, "Before daemonize\n");
    daemonize();
    PRINTF(LEVEL_ERROR, "After daemonize\n");
    
    printf("makefile test end.\n");
    
	return 0;	
}

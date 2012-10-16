#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "defs.h"
#include "log.h"
#include "thread.h"

static CS_T g_log_cs;
static FILE *g_LogFile = NULL;

static INT32 log_init()
{
	if (NULL == g_LogFile)
	{
		g_LogFile = fopen("log.log", "a");
		
		if (NULL == g_LogFile)
		{
			return R_ERROR;
		}
	}
	
	CS_INIT(&g_log_cs);
	
	return R_OK;
}

INT32 PRINTF(UINT64 mode, INT8 *format, ...)
{
    static INT8     str[1024] = {0};
    va_list         arg;
    
    if (NULL == g_LogFile)
    {
    	if (R_ERROR == log_init())
    	{
    		printf("init log file error\n");
	    	return R_ERROR;
	    }
    }
    
    CS_ENTER(&g_log_cs);
    
#ifndef _WIN32
	if (0 == (stdout->_IO_file_flags & _IO_FULL_BUF))
    {
        fprintf(stdout, "\033[%d;49;%dm", (INT8)((mode & TEXT_MASK) >> TEXT_POS), 30 + (INT8)((mode & COLOR_MASK) >> COLOR_POS));
    }
#endif

    /*if (TIME_SHOW == (mode & TIME_MASK))
    {
        INT8 cur_time[128];
        struct tm ptm = {0};
        time_t t = time(NULL);
        localtime_r(&t, &ptm);
        strftime(cur_time, 128, "[%H:%M:%S %Y-%m-%d ] ",&ptm);
        fprintf(stdout, "%s", cur_time); 
    }*/
    
    va_start(arg, format);
    vfprintf(g_LogFile, format, arg);
    va_end(arg);
        
    /*if(LogLevel <= INFORM_LEVEL || ((g_DbgMod & ModSeq) != 0))
    {

        va_list Arg;
        time_t Now;
        struct tm *TimeNow;
        CHAR *TimeStr;
        time(&Now);
        TimeNow = localtime(&Now);
        TimeStr = asctime(TimeNow);
        TimeStr[24] = '\0';
        printf("%s: %s: DPI/", TimeStr, g_DbgStr[LogLevel - ALARM_LEVEL]);
        va_start(Arg, Format);
        vprintf(Format, Arg);
        va_end(Arg);
        if(g_ShellOk == 1)
        {
            va_start(Arg, Format);
            vsprintf(Str, Format, Arg);
            va_end(Arg);
            if(LogLevel > ERROR_LEVEL)
                ztsh_logout(DPI_DEBUG, "%s", Str);
            if (g_LogOn == LOG_ON)
            {
                sprintf(Str, "%s: %s: DPI/", TimeStr, g_DbgStr[LogLevel - ALARM_LEVEL]);
                va_start(Arg, Format);
                vsprintf(Str + strlen(Str), Format, Arg);
                va_end(Arg);
                write(g_LogFile, Str, strlen(Str));
            }
        }
        
    }*/
    
#ifndef _WIN32
	if (0 == (stdout->_IO_file_flags & _IO_FULL_BUF))
    {
        printf("\033[0m");
    }
#endif
	
    CS_LEAVE(&g_log_cs);
    
    return 0;
}

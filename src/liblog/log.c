#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <limits.h>
#include <string.h>

#include "defs.h"
#include "log.h"
#include "thread.h"

static const UINT8 g_debug_str[8][8] = {
    "TESTS",
    "DEBUG",
    "INFOM",
    "ALARM",
    "WARNS",
    "ERROR",
    "UNDEF",
    "UNDEF",
};

static CS_T g_log_cs;
static FILE *g_logfile = NULL;
static UINT8 g_dlevel = 1;

static INT32 log_init()
{
    UINT8           log_file_name[PATH_MAX] = {0};
    time_t          log_t = 0;
    struct  tm      log_tm = {0};
    
    if (NULL == g_logfile)
    {
        memset((void*)log_file_name,0x00,PATH_MAX);
        time( &log_t);
        localtime_r(&log_t, &log_tm);
        sprintf(log_file_name, "zthfs-%04d-%02d-%02d-%02d-%02d-%02d.log", log_tm.tm_year+1900,
                log_tm.tm_mon + 1, log_tm.tm_mday, log_tm.tm_hour, log_tm.tm_min, log_tm.tm_sec);
        
        g_logfile = fopen(log_file_name, "a");
        
        if (NULL == g_logfile)
        {
            return R_ERROR;
        }
        else
        {
            CS_INIT(&g_log_cs);
            PRINTF(LEVEL_INFORM, "open log file %s ok.\n", log_file_name);
        }
    }
    
    return R_OK;
}

INT32 PRINTF(UINT64 mode, char *format, ...)
{
    INT8        str[1024] = {0};
    UINT64      dlevel = -1;
    va_list     arg;
    
    // ensure logfile has been opened
    if (NULL == g_logfile)
    {
        if (R_ERROR == log_init())
        {
            printf("init log file error\n");
            return R_ERROR;
        }
    }
    
    dlevel = mode & LEVEL_MASK;
    if (LEVEL_ERROR == dlevel)
    {
        mode &= COLOR_MASK;
        mode |= COLOR_RED;
    }
        
    CS_ENTER(&g_log_cs);
    
    // show different color for shell console
    if (stdout->_IO_file_flags & _IO_LINE_BUF)
    {
        INT8 color = (INT8)((mode & COLOR_MASK) >> COLOR_POS);
        if (0 == color) color = 9;
        fprintf(stdout, "\033[%d;49;%dm", (INT8)((mode & TEXT_MASK) >> TEXT_POS), 30 + color);
    }

    // show time
    if (TIME_SHOW == (mode & TIME_MASK))
    {
        INT8 cur_time[128];
        struct tm ptm = {0};
        time_t t = time(NULL);

        localtime_r(&t, &ptm);
        strftime(cur_time, 128, "[%H:%M:%S %Y-%m-%d]",&ptm);
        fprintf(stdout, "%s", cur_time);
        fprintf(g_logfile, "%s", cur_time);
    }
    
    // show debug level info
    fprintf(g_logfile, "[%s]", g_debug_str[dlevel >> LEVEL_POS]);
    fprintf(stdout, "[%s]", g_debug_str[dlevel >> LEVEL_POS]); 
    
    va_start(arg, format);
    vfprintf(stdout, format, arg);
    va_end(arg);
    va_start(arg, format);
    vfprintf(g_logfile, format, arg);
    va_end(arg);
    
    if (stdout->_IO_file_flags & _IO_LINE_BUF)
    {
        fprintf(stdout, "\033[0m");
    }
    
    fflush(stdout);
    fflush(g_logfile);
    
    CS_LEAVE(&g_log_cs);
    
    return 0;
}

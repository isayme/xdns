#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "liblog.h"
#include "libconfig.h"

INT32 get_cfg_from_file(char *key, char *value, INT32 value_len, char *cfg_path)
{
    FILE *fp = NULL;
    char *line = NULL;
    char *real_line = NULL;
    char *tmp = NULL;
    size_t len = 0;
    size_t read;

    if (NULL == key || NULL == value || 0 == value_len || NULL == cfg_path) {
        PRINTF(LEVEL_ERROR, "%s argument error.\n", __func__);
        return -1;
    }

    memset(value, 0, value_len);
    fp = fopen(cfg_path, "rb");
    if (NULL == fp) {
        PRINTF(LEVEL_ERROR, "%s fopen error, path = [%s].\n", __func__, cfg_path);
        return -1;
    }

    while ((read = getline(&line, &len, fp)) != -1) {
        PRINTF(LEVEL_TEST, "%d:[%s]\n", read, line);

        real_line = line;

        // skip white-space characters
        while (0 != isspace((INT32)*(real_line))) real_line++;

        // skip comment line
        if ('#' == real_line[0]) continue;

        tmp = strstr(real_line, key);

        // skip if key not found
        if (NULL == tmp) continue;

        // skip key to parse key value
        real_line = tmp + strlen(key);

        // skip white-space characters
        while (0 != isspace((INT32)*(real_line))) real_line++;
        // if next character is not `=`, skip this line
        if ('=' != real_line[0])
            continue;
        else
            real_line++;
        // skip white-space characters
        while (0 != isspace((INT32)*(real_line))) real_line++;
        
        // calc key value
        tmp = real_line;
        while (0 != isprint((INT32)*(real_line)) && 0 == isspace((INT32)*(real_line))) real_line++;
        
        if ((real_line == tmp) || ((real_line - tmp) > value_len)) {
            PRINTF(LEVEL_WARNING, "value buffer not enough or no valid value exsit.\n");
            continue;
        }

        strncpy(value, tmp, real_line - tmp);
        value[real_line - tmp] = '\0';

        PRINTF(LEVEL_TEST, "key:%s\tvalue:%s\n", key, value);
        fclose(fp);
        if (line) free(line);
        return 0;
    }

    fclose(fp);
    if (line) free(line);

    return -1;
}

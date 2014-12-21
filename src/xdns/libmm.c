#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "libthread.h"
#include "libmm.h"

buf_ctrl_t *buf_create(INT32 blk_cnt, INT32 blk_size)
{
    buf_ctrl_t* buf;
    INT32 i;

    if ((blk_cnt <= 0) || (blk_size <= 0))
        return NULL;

    buf = (buf_ctrl_t *)malloc(sizeof(buf_ctrl_t));
    if (NULL == buf)
        return NULL;

    buf->buf_size = blk_cnt;
    buf->buf_unit = blk_size+sizeof(void*);

    buf->buf = (UINT8 *)malloc(buf->buf_size * buf->buf_unit);
    if (NULL == buf->buf) {
        free(buf);
        return NULL;
    }

    buf->que = (UINT32*)malloc((blk_cnt+1)*sizeof(UINT32));
    if (NULL == buf->que) {
        free(buf->buf);
        free(buf);
        return NULL;
    }

    buf->ic = 0;
    buf->oc = 0;
    buf->head = 0;
    buf->tail = blk_cnt;

    for(i=0;i<blk_cnt;i++) {
        *(buf_ctrl_t**)(buf->buf+i*buf->buf_unit) = NULL;
        buf->que[i] = i;
    }
    buf->que[blk_cnt] = -1;

    CS_INIT(&buf->cs);

    return buf;
}

INT32 buf_destroy(buf_ctrl_t* buf)
{
    if (buf == NULL)
        return 0;

    if (buf->buf != NULL)
        free(buf->buf);
    if (buf->que != NULL)
        free(buf->que);
    CS_DEL(&buf->cs);
    free(buf);

    return 0;
}

UINT8* buf_get(buf_ctrl_t* buf)
{
    size_t  idx;
    UINT8*  blk;

    if (buf == NULL)
        return NULL;

    CS_ENTER(&buf->cs);
    if (buf->head == buf->tail) {
        CS_LEAVE(&buf->cs);
        return NULL;
    } else {
        idx = buf->head++;
        if (buf->head == buf->buf_size+1)
            buf->head = 0;
        ++buf->oc;
        blk = buf->buf + *(buf->que+idx)*buf->buf_unit;
        *(buf_ctrl_t**)blk = buf;
        blk += sizeof(void*);
        CS_LEAVE(&buf->cs);
        return blk;
    }
}

INT32 buf_ret(UINT8* blk)
{
    buf_ctrl_t* buf;
    UINT32 idx;
    size_t  tt;

    if (blk == NULL)
        return -1;

    buf = *(buf_ctrl_t**)(blk-sizeof(void*));
    if (buf == NULL)
        return -2;

    idx = (INT32)((blk - buf->buf)/buf->buf_unit);
    if ((idx < 0) || (idx > buf->buf_size))
        return -3;

    CS_ENTER(&buf->cs);
    tt = buf->tail + 1;
    if (tt == buf->buf_size+1)
        tt = 0;
    if (tt == buf->head) {
        CS_LEAVE(&buf->cs);
        return -1;
    } else {
        *(buf->que+buf->tail) = idx;
        buf->tail = tt;
        ++buf->ic;
        *(buf_ctrl_t**)(blk-sizeof(void*)) = NULL;
        CS_LEAVE(&buf->cs);
        return 0;
    }
}
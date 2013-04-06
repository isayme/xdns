#ifndef _LIBMM_H
#define _LIBMM_H

#include "defs.h"
#include "libthread.h"

#pragma pack(1)

typedef struct buf_ctrl_t {
    size_t  buf_size;
    size_t  buf_unit;
    UINT8*  buf;
    UINT64  ic;
    UINT64  oc;
    size_t  head;
    size_t  tail;
    UINT32* que;
    CS_T    cs;
}buf_ctrl_t;

#pragma pack()

buf_ctrl_t *buf_create(INT32 blk_cnt, INT32 blk_size);
INT32 buf_destroy(buf_ctrl_t* buf);

UINT8* buf_get(buf_ctrl_t* buf);
INT32 buf_ret(UINT8* blk);

#endif
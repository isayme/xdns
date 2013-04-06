#ifndef _XDNS_H
#define _XDNS_H

#include "defs.h"

#define XDNS_DNS_PORT 53
#define XDNS_CFG_FILE "xdns.cfg"

#pragma pack(1)

typedef enum xdns_state_t {
    XDNS_STATE_PREPARE,
    XDNS_STATE_RUNNING,
    XDNS_STATE_STOP
}xdns_state_t;

typedef struct xdns_config_t {
    UINT8 daemon;
    UINT8 dbg_level;

#define XDNS_CFG_TP_DEF_NUM 50
    INT32 tp_num;
#define XDNS_CFG_FILE_LEN 1024 //PATH_MAX
    UINT8 cfg_path[XDNS_CFG_FILE_LEN];


}xdns_config_t;

typedef struct xdns_tp_arg_t {
    struct sockaddr addr;
    int sockfd;
#define XDNS_DNS_MSG_LEN 1024
    char msg[XDNS_DNS_MSG_LEN];
    int msg_len;
}xdns_tp_arg_t;

typedef struct xdns_srv_info_t {
#define XDNS_SRV_ADDR_LEN 32
    char addr[XDNS_SRV_ADDR_LEN];
}xdns_srv_info_t;

typedef struct xdns_srv_t {
    xdns_srv_info_t *srvs;
    INT32 num;
}xdns_srv_t;

#pragma pack()

#endif

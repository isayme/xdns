#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "defs.h"
#include "libthreadpool.h"
#include "liblog.h"
#include "libudp.h"

#include "libdaemon.h"
#include "libconfig.h"

#include "libmm.h"
#include "xdns.h"

static xdns_state_t g_state = XDNS_STATE_PREPARE;
static xdns_config_t g_config = {0};

static xdns_srv_t g_dns_srv;

static thread_pool_t *g_threadpool = NULL;
static buf_ctrl_t *g_buf_ctrl = NULL;

static void xdns_help();
static INT32 xdns_check_para(int argc, char **argv);

static INT32 xdns_get_config();

static void xdns_tp_func(void *arg);
static int xdns_callback(struct sockaddr addr, char *buff, int blen);

int main(int argc, char **argv)
{
    if (-1 == xdns_check_para(argc, argv)) {
        PRINTF(LEVEL_ERROR, "check para error.\n");
        return -1;
    }

    PRINTF(LEVEL_INFORM, "xdns start.\n");

    if (-1 == xdns_get_config()) {
        PRINTF(LEVEL_ERROR, "get config error.\n");
        goto _err;
    }
    PRINTF(LEVEL_INFORM, "get config ok.\n");

    g_buf_ctrl = buf_create(g_config.tp_num, sizeof(xdns_tp_arg_t));
    if (NULL == g_buf_ctrl) {
        PRINTF(LEVEL_ERROR, "buf create error.\n");
        goto _err;
    }
    PRINTF(LEVEL_INFORM, "buf create ok.\n");

    if (NULL == (g_threadpool = tp_create(g_config.tp_num))) {
        PRINTF(LEVEL_ERROR, "thread pool init error.\n");
        goto _err;
    }
    sleep(1);
    PRINTF(LEVEL_INFORM, "thread pool init ok.\n");

    if (-1 == udp_init(XDNS_DNS_PORT, xdns_callback)) {
        PRINTF(LEVEL_ERROR, "udp init error.\n");
        goto _err;
    }
    PRINTF(LEVEL_INFORM, "udp init ok.\n");

    while (XDNS_STATE_STOP != g_state) {
        sleep(1);
    }

_err:
    if (NULL != g_threadpool) {
        tp_destroy(g_threadpool);
    }
    if (NULL != g_buf_ctrl) {
        buf_destroy(g_buf_ctrl);
    }
    if (NULL != g_dns_srv.srvs) {
        free(g_dns_srv.srvs);
    }
    PRINTF(LEVEL_INFORM, "xdns exit.\n");
    return 0;
}

static void xdns_help()
{
    printf("Usage : xdns [OPTIONS]\n");
    printf("  -c <file_path>      config file path\n");
    printf("  -d <Y|y>            run as a daemon if 'Y' or 'y', otherwise not.\n");
    printf("  -t <number>         thread pool number\n");
    printf("  -l <level>          debug level, range [0 - 5].\n");
    printf("  -h                  help information\n");
}

static INT32 xdns_check_para(int argc, char **argv)
{
    int ch;

    g_config.daemon = 0;
    g_config.dbg_level = 1;
    g_config.tp_num = XDNS_CFG_TP_DEF_NUM;
    strncpy((char *)g_config.cfg_path, XDNS_CFG_FILE, XDNS_CFG_FILE_LEN);

    while ((ch = getopt(argc, argv, ":d:c:l:t:h")) != -1) {
        switch (ch) {
            case 'd':
                if (0 == strcmp(optarg, "Y") || 0 == strcmp(optarg, "y")) {
                    g_config.daemon = 1;
                    daemonize();
                    PRINTF(LEVEL_INFORM, "run as daemon.\n");
                }
                break;
            case 'c':
                strncpy((char *)(g_config.cfg_path), optarg, XDNS_CFG_FILE_LEN);
                if (0 != access((char *)(g_config.cfg_path), 0)) {
                    PRINTF(LEVEL_ERROR, "config file [%s] cannot access.\n", g_config.cfg_path);
                    return -1;
                }
                break;
            case 'l':
                if (0 <= atoi(optarg) && 5 >= atoi(optarg)) {
                    g_config.dbg_level = atoi(optarg);
                    liblog_level(g_config.dbg_level);
                } else {
                    printf("level [%d] ot of range [0 - 5].\n", atoi(optarg));
                }
                break;
            case 't':
                if (0 >= atoi(optarg)) {
                    g_config.tp_num = XDNS_CFG_TP_DEF_NUM;
                    break;
                }
                g_config.tp_num = atoi(optarg);
                break;
            case 'h':
            default:
                xdns_help();
                exit(0);
                break;
        }
    }

    return 0;
}

static INT32 xdns_get_config()
{
    char key[XDNS_SRV_ADDR_LEN];
    char value[XDNS_SRV_ADDR_LEN];
    int i;

    snprintf(key, XDNS_SRV_ADDR_LEN, "xdns.num");
    if (-1 == get_cfg_from_file(key, value, XDNS_SRV_ADDR_LEN, (char *)g_config.cfg_path)) {
        PRINTF(LEVEL_ERROR, "get config {%s} error.\n", key);
        return -1;
    } else {
        g_dns_srv.num = atoi(value);
        if (0 >= g_dns_srv.num) {
            PRINTF(LEVEL_ERROR, "config {%s} value [%d] not valid.\n", key, g_dns_srv.num);
            return -1;
        }

        g_dns_srv.srvs = malloc(sizeof(xdns_srv_info_t) * g_dns_srv.num);
        if (NULL == g_dns_srv.srvs) {
            PRINTF(LEVEL_ERROR, "apply memory error.\n");
            return -1;
        }
    }
    
    for (i = 0; i < g_dns_srv.num; i++) {
        snprintf(key, XDNS_SRV_ADDR_LEN, "xdns.srv[%d]", i);
        if (-1 == get_cfg_from_file(key, g_dns_srv.srvs[i].addr, XDNS_SRV_ADDR_LEN, (char *)g_config.cfg_path)) {
            PRINTF(LEVEL_ERROR, "get config {%s} error.\n", key);
            free(g_dns_srv.srvs);
            return -1;
        }
    }

    return 0;
}

static void xdns_tp_func(void *arg)
{
    xdns_tp_arg_t *tp_arg = (xdns_tp_arg_t *)arg;
    //int sockfd = 0;
    int i, cnt;
    struct timeval tv = {2, 0};  // 超时设置
    struct sockaddr_in s_addr = {0};    // 服务器地址

    struct sockaddr client;
    int clen = sizeof(client);

    static int dns_idx = 0; 

    if (0 == tp_arg->sockfd) {
        PRINTF(LEVEL_DEBUG, "sock init.\n");
        if ((tp_arg->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                PRINTF(LEVEL_DEBUG, "socket error [%d]\n", errno);
                goto _err;
        }

        if (0 != setsockopt(tp_arg->sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv))) {
            PRINTF(LEVEL_ERROR, "set timeout error.\n");
            goto _err;
        }
        if (0 != setsockopt(tp_arg->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))) {
            PRINTF(LEVEL_ERROR, "set timeout error.\n");
            goto _err;
        }
    }
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = inet_addr(g_dns_srv.srvs[dns_idx].addr);
    s_addr.sin_port = htons(XDNS_DNS_PORT);

    for (i = 0; i < tp_arg->msg_len; i++) {
        tp_arg->msg[i] = ~tp_arg->msg[i];
    }

    cnt = sendto(tp_arg->sockfd, tp_arg->msg, tp_arg->msg_len, 0, (struct sockaddr *)&s_addr, sizeof(s_addr));
    if (cnt == -1) {
        PRINTF(LEVEL_DEBUG, "sendto [%s] error.\n", g_dns_srv.srvs[dns_idx].addr);

        dns_idx = (dns_idx + 1) % g_dns_srv.num;
        s_addr.sin_addr.s_addr = inet_addr((char *)g_dns_srv.srvs[dns_idx].addr);
        cnt = sendto(tp_arg->sockfd, tp_arg->msg, tp_arg->msg_len, 0, (struct sockaddr *)&s_addr, sizeof(s_addr));
        if (cnt == -1) {
            PRINTF(LEVEL_DEBUG, "sendto [%s] error.\n", g_dns_srv.srvs[dns_idx].addr);
            dns_idx = (dns_idx + 1) % g_dns_srv.num;
            goto _err;
        }
    }
    dns_idx = (dns_idx + 1) % g_dns_srv.num;

    tp_arg->msg_len = recvfrom(tp_arg->sockfd, tp_arg->msg, XDNS_DNS_MSG_LEN, 0, (struct sockaddr *)&client, (socklen_t *)&clen);
    if (tp_arg->msg_len < 0) {
        PRINTF(LEVEL_DEBUG, "recvfrom error!\n");
        goto _err;
    }

    udp_reply(tp_arg->addr, tp_arg->msg, tp_arg->msg_len);

_err:
    buf_ret(arg);
    //close(sockfd);
    return;
}

static int xdns_callback(struct sockaddr addr, char *buff, int blen)
{
    xdns_tp_arg_t *tp_arg = NULL;

    while (NULL == tp_arg) {
        tp_arg = (xdns_tp_arg_t *)buf_get(g_buf_ctrl);
        if (NULL == tp_arg) {
            PRINTF(LEVEL_WARNING, "no buf left.\n");
            usleep(500 * 1000);
        }
    }
    
    memcpy(&tp_arg->addr, &addr, sizeof(addr));
    tp_arg->msg_len = blen;

    memcpy(tp_arg->msg, buff, blen);

    while (-1 == tp_add_task(g_threadpool, xdns_tp_func, (void *)tp_arg)) {
        PRINTF(LEVEL_WARNING, "thread pool add task error.\n");
        usleep(500 * 1000);
    }

    return 0;
/*
    int i, cnt;
    struct timeval tv = {2, 0};  // 超时设置

    int sockfd = 0;
    struct sockaddr_in s_addr = {0};
    static int dns_idx = 0;
    static char dns_rsp[1024];
    struct sockaddr client;
    int clen = sizeof(client);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
            PRINTF(LEVEL_DEBUG, "socket error!\n");
            return -1;
    }

    if (0 != setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)))
    {
        PRINTF(LEVEL_ERROR, "set timeout error.\n");
        return -1;
    }
    if (0 != setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)))
    {
        PRINTF(LEVEL_ERROR, "set timeout error.\n");
        return -1;
    }

    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = inet_addr(g_dns_srvs[dns_idx]);
    s_addr.sin_port = htons(XDNS_DNS_PORT);

    for (i = 0; i < blen; i++)
    {
        buff[i] = ~buff[i];
    }

    cnt = sendto(sockfd, buff, blen, 0, (struct sockaddr *)&s_addr, sizeof(s_addr));
    if (cnt == -1)
    {
        PRINTF(LEVEL_DEBUG, "sendto [%s] error.\n", g_dns_srvs[dns_idx]);

        dns_idx = (dns_idx + 1) % XDNS_SRV_NUM;
        s_addr.sin_addr.s_addr = inet_addr(g_dns_srvs[dns_idx]);
        cnt = sendto(sockfd, buff, blen, 0, (struct sockaddr *)&s_addr, sizeof(s_addr));
        if (cnt == -1)
        {
            PRINTF(LEVEL_DEBUG, "sendto [%s] error.\n", g_dns_srvs[dns_idx]);
            dns_idx = (dns_idx + 1) % XDNS_SRV_NUM;
            return -1;
        }
    }
    dns_idx = (dns_idx + 1) % XDNS_SRV_NUM;

    cnt = recvfrom(sockfd, dns_rsp, 1024, 0, (struct sockaddr *)&client, &clen);
    if (cnt < 0)
    {
        PRINTF(LEVEL_DEBUG, "recvfrom error!\n");
        return -1;
    }

    udp_reply(addr, dns_rsp, cnt);

    return 0;
    */
}

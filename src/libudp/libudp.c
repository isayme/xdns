#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "libudp.h"
#include "defs.h"

static SOCK_INFO g_sockinfo = {0};

static void udp_callback(void *arg);

int udp_init(UINT16 port, void *func)
{
    int sockfd;
    struct sockaddr_in addr;
    
    int ret;
    pthread_t ntid;
    pthread_attr_t attr;

    if (0 != g_sockinfo.sockfd || NULL != g_sockinfo.func)
    {
        dprintf("udp inited before, now close that one!\n");
        close(g_sockinfo.sockfd);
        memset((void*)&g_sockinfo, 0, sizeof(g_sockinfo));
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
            dprintf("socket error!\n");
            return R_ERROR;
    }

    if (0 < port)
    {
        bzero((void *)&addr, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);
        
        if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
        {
            printf("bind error!\n");
            close(sockfd);
            return R_ERROR;
        }
    }

    if (0 != pthread_attr_init(&attr))
    {
        dprintf("pthread_attr_init error!\n");
        close(sockfd);
        return R_ERROR;
    }
    if (0 != pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))
    {
        dprintf("pthread_attr_setdetachstate error!\n");
        close(sockfd);
        return R_ERROR;
    }
    ret = pthread_create(&ntid, &attr, (void*)udp_callback, NULL);
    if (0 != ret)
    {
        dprintf("pthread_create error!\n");
        close(sockfd);
        return R_ERROR;
    }

    g_sockinfo.state = STATE_RUN;
    g_sockinfo.sockfd = sockfd;
    g_sockinfo.func = func;

    return sockfd;
}

int udp_uninit()
{
    g_sockinfo.state = STATE_STOP;
    close(g_sockinfo.sockfd);
    dprintf("udp uninit ok.\n");

    return R_OK;
}

int udp_reply(struct sockaddr addr, UINT8 *buff, UINT16 blen)
{
    int cnt;

    assert(buff);
    assert(blen);

    if (0 == g_sockinfo.sockfd || NULL == g_sockinfo.func || STATE_RUN != g_sockinfo.state)
    {
        return R_ERROR;
    }

    cnt = sendto(g_sockinfo.sockfd, buff, blen, 0, (struct sockaddr *)&addr, sizeof(addr));
    if (cnt == -1)
    {
        return R_ERROR;
    }

    return cnt;
}

int udp_send(UINT8 *dip, UINT16 dport, UINT8 *buff, UINT16 blen)
{
    struct sockaddr_in addr;
    int cnt;

    assert(buff);
    assert(blen);

    if (0 == g_sockinfo.sockfd || NULL == g_sockinfo.func || STATE_RUN != g_sockinfo.state)
    {
        return R_ERROR;
    }

    bzero((void *)&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(dip);
    addr.sin_port = htons(dport);

    cnt = sendto(g_sockinfo.sockfd, buff, blen, 0, (struct sockaddr *)&addr, sizeof(addr));
    if (cnt == -1)
    {
        return R_ERROR;
    }

    return cnt;
}

static void udp_callback(void *arg)
{
    int cnt;
    #define UDP_BUFF_LEN 1024
    char buff[UDP_BUFF_LEN];
    int blen = UDP_BUFF_LEN;

    struct sockaddr client;
    int clen = sizeof(client);

    while (0 == g_sockinfo.sockfd || NULL == g_sockinfo.func || STATE_UNINIT == g_sockinfo.state)
    {
        usleep(1000 * 10);
        cnt++;
        if (cnt > 10)
        {
            dprintf("pthread_create error!\n");
            return;
        }
    }
    dprintf("start ok\n");

    while (STATE_RUN == g_sockinfo.state)
    {

        bzero(buff, blen);
        cnt = recvfrom(g_sockinfo.sockfd, buff, blen, 0, (struct sockaddr *)&client, &clen);
        if (cnt < 0)
        {
            dprintf("recvfrom error!\n");
        }

        ((CALLBACK)g_sockinfo.func)(client, buff, cnt);
    }

    dprintf("exit ok\n");
    return;
}
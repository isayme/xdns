#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "libthreadpool.h"
#include "liblog.h"

static void *tp_wrapper_fn(void * arg)
{
    tp_worker_t *thread = (tp_worker_t *)arg;
    thread_pool_t *pool = thread->parent;
    
    PRINTF(LEVEL_DEBUG, "%s %d start.\n", __func__, thread->tid);

    while (1)
    {
        pthread_mutex_lock(&thread->lock);
        pthread_cond_wait(&thread->cond, &thread->lock);
        PRINTF(LEVEL_DEBUG, "thread pool thread[%d] deal job.\n", thread->id);
        
        thread->func(thread->arg);
        pthread_mutex_unlock(&thread->lock);
        
        
        pthread_mutex_lock(&pool->lock);
        pool->t_tail->next = thread;
        pool->t_tail = thread;
        pool->t_tail->next = NULL;
        pthread_mutex_unlock(&pool->lock);
    }
    
    
    PRINTF(LEVEL_DEBUG, "%s %d exit.\n", __func__, thread->tid);
    pthread_exit(0);
}

thread_pool_t *tp_create(int t_num)
{
    int i;
    int th_create = 0;
    thread_pool_t *tp = NULL;
    
    if (0 >= t_num)
    {
        goto _err;
    }

    tp = (thread_pool_t *)malloc(sizeof(thread_pool_t));
    if (NULL == tp)
    {
        goto _err;
    }
    
    memset((void*)tp, 0, sizeof(thread_pool_t));
    pthread_mutex_init(&tp->lock, NULL);
    //pthread_cond_init(&tp->cond, NULL);
    tp->t_num = t_num;
    
    tp->t_worker = (tp_worker_t *)malloc(sizeof(tp_worker_t) * (t_num + 1));
    if (NULL == tp->t_worker)
    {
        goto _err;
    }
    else
    {
        tp->t_head = &tp->t_worker[0];
        tp->t_tail = &tp->t_worker[t_num];
        tp->t_tail->next = NULL;
        for (i = 0; i < t_num; i++)
        {
            tp->t_worker[i].next = &tp->t_worker[i+1];
        } 
    }
    
    for (i = 0; i < (t_num + 1); i++)
    {
        pthread_mutex_init(&tp->t_worker[i].lock, NULL);
        pthread_cond_init(&tp->t_worker[i].cond, NULL);
        tp->t_worker[i].parent = tp;
        tp->t_worker[i].id = i;
        
        if( 0 != pthread_create(&tp->t_worker[i].tid, NULL, tp_wrapper_fn, &tp->t_worker[i]))    
		{
		    goto _err;
		}
		th_create++;
    }

    PRINTF(LEVEL_DEBUG, "thread_pool create ok.\n");
    return tp;

_err:
    for (i = 0; i < th_create; i++)
    {
        pthread_mutex_lock(&tp->t_worker[i].lock);
        tp->t_worker[i].func = pthread_exit;
        tp->t_worker[i].arg = 0;
        pthread_cond_signal(&tp->t_worker[i].cond);  
        pthread_mutex_unlock(&tp->t_worker[i].lock);

        pthread_join(tp->t_worker[i].tid, NULL);
    }
    
    if (NULL != tp)
    {
        if (NULL != tp->t_worker)
        {
            free(tp->t_worker);
        }
        free(tp);
    }
    PRINTF(LEVEL_DEBUG, "thread_pool create error.\n");
    return NULL;
}


INT32 tp_add_task(thread_pool_t *tp, tp_func fn, void *arg)
{
    tp_worker_t *worker;
    
    if (NULL == tp || NULL == fn)
    {
        PRINTF(LEVEL_ERROR, "%s arguments error.\n", __func__);
        return R_ERROR;
    }
        
    if (tp->t_tail == tp->t_head)
    {
        PRINTF(LEVEL_DEBUG, "thread_pool has no idle thread left.\n");
        return R_ERROR;
    }
    
    pthread_mutex_lock(&tp->lock);
    worker = tp->t_head;
    worker->func = fn;
    worker->arg = arg;
    tp->t_head = tp->t_head->next;
    pthread_mutex_unlock(&tp->lock);
    
    pthread_mutex_lock(&worker->lock);
    pthread_cond_signal(&worker->cond);  
    pthread_mutex_unlock(&worker->lock);
    
    
    PRINTF(LEVEL_DEBUG, "thread_pool add a job ok.\n");
    return R_OK;
}

INT32 tp_destroy(thread_pool_t *tp)
{
    int i;
    
    if (NULL == tp)
    {
        PRINTF(LEVEL_ERROR, "argument NULL.\n");
        return R_ERROR;
    }
        
    for (i = 0; i < (tp->t_num + 1); i++)
    {
        pthread_mutex_lock(&tp->t_worker[i].lock);
        tp->t_worker[i].func = pthread_exit;
        tp->t_worker[i].arg = 0;
        pthread_cond_signal(&tp->t_worker[i].cond);  
        pthread_mutex_unlock(&tp->t_worker[i].lock);
        
        PRINTF(LEVEL_DEBUG, "thread_pool join thread[%d].\n", tp->t_worker[i].id);
        if (0 != pthread_join(tp->t_worker[i].tid, NULL))
        {
            PRINTF(LEVEL_WARNING, "thread_pool thread[%x] exit error.\n", tp->t_worker[i].id);
        }
    }
    
    free(tp->t_worker);
    free(tp);
    tp = NULL;
    
    PRINTF(LEVEL_DEBUG, "thread_pool destroy ok.\n");
    return R_OK;
}
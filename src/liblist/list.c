#include <stdio.h>
#include "defs.h"
#include "list.h"

/*
 *  add "new" between "head" and "list"
 */
static void __list_add(list_head *new, list_head *head, list_head *tail)
{
    new->next = tail;
    new->prev = head;

    head->next = new;
    tail->prev = new;
}

/*
 *  add "new" before "head"
 */
int list_add(list_head *new, list_head *head)
{
    if (NULL == new || NULL == head)
    {
        _perror();
        return R_ERROR;
    }
    
    __list_add(new, head, head->next);

    return R_OK;
}

/*
 *  add "new" after "head"
 */
int list_add_tail(list_head *new, list_head *head)
{
    if (NULL == new || NULL == head)
    {
        _perror();
        return R_ERROR;
    }
    
    __list_add(new, head->prev, head);

    return R_OK;
}

/*
 *  del node between "prev" and "next"
 */
static void __list_del(list_head *prev, list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

#define LIST_POISON1  ((void *) 0x00100100)
#define LIST_POISON2  ((void *) 0x00200200)

/*
 *  out "entry"
 */
int list_delete(list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = LIST_POISON1;
    entry->prev = LIST_POISON2;
}

/*
 *  is list empty
 */
int list_empty(const list_head *head)
{
    return (head->next == head);
}

#ifndef __LIST_H
#define __LIST_H

typedef struct _list_head{
    struct _list_head *next;
    struct _list_head *prev;
}list_head;

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { \
    (ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); \
            pos = pos->next)

#define offsetof(TYPE, MEMBER)   ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) (type *)((char *)ptr - offsetof(type,member))

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

int list_add(list_head *new, list_head *head);
int list_add_tail(list_head *new, list_head *head);
int list_delete(list_head *entry);
int list_empty(const list_head *head);

#endif
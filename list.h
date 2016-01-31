#pragma once
#include <stdlib.h>
#include <stddef.h>


typedef enum {
  LIST_OK,
  LIST_ERR
} ListStatus;

typedef struct ListNode {
  struct ListNode *prev;
  void *next;
} ListNode;

typedef struct {
  ListNode root;
  size_t size;
  size_t offset;
} List;


#define list_node_init(P, T, M) list_node_init_with_offset (P, offsetof(T, M))
#define list_new(T, M) list_new_with_offset (offsetof(T, M))

#define list_head(L) list_next(L, L)
#define list_tail(L) list_prev(L, L)
#define list_pop_head(L) list_remove(l, l->root.next)
#define list_pop_tail(L) list_remove(l, l->root.prev->prev->next)

#define LIST_FOREACH(T, V, L) for(T *V = list_head(L); \
                                  V != NULL; \
                                  V = list_next(L, V))

#define LIST_FOREACH_REV(T, V, L) for (T *V = list_tail(L); \
                                       V != NULL; \
                                       V = list_prev(L, V))


void list_node_init_with_offset (ListNode *n, size_t offset);
ListStatus list_node_unlink (List *l, ListNode *node);
List *list_new_with_offset (size_t offset);
ListStatus list_free (List *l);
void *list_next (List *l, void *elem);
void *list_prev (List *l, void *elem);
ListStatus list_append (List *l, void *elem);
ListStatus list_prepend (List *l, void *elem);
ListStatus list_append_after (List *l, void *target, void *elem);
ListStatus list_append_before (List *l, void *target, void *elem);
void *list_remove (List *l, void *elem);

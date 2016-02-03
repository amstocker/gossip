#include "list.h"


#define NODE_FROM_ELEM(L, E) ((E == (void*) L) \
                             ? &L->root \
                             : (ListNode*) ((size_t) E + L->offset))

#define ELEM_FROM_NODE(L, N) ((N == &L->root) \
                             ? L \
                             : (void*) ((size_t) N - L->offset))

static inline void insert_between (List*, ListNode*, ListNode*, void*);


void
list_node_init_with_offset (ListNode *n, size_t offset)
{
  n->prev = n;
  n->next = (void*) ((size_t) n - offset);
}


ListStatus
list_node_unlink (List *l, ListNode *node)
{
  if (!node || node == &l->root)
    return LIST_ERR;
  node->prev->next = node->next;
  NODE_FROM_ELEM(l, node->next)->prev = node->prev;
  list_node_init_with_offset (node, l->offset);
  l->size--;
  return LIST_OK;
}


List*
list_new_with_offset (size_t offset)
{
  List *l = malloc (sizeof(List));
  if (!l)
    return NULL;
  l->size = 0;
  l->offset = offset;
  list_node_init_with_offset (&l->root, offsetof(List, root));
  return l;
}


ListStatus
list_free (List *l)
{
  if (!l)
    return LIST_ERR;
  free(l);
  return LIST_OK;
}


void*
list_next (List *l, void *elem)
{
  ListNode *n = NODE_FROM_ELEM(l, elem);
  if (n->next == (void*) l)
    return NULL;
  return n->next;
}

void*
list_prev (List *l, void *elem)
{
  ListNode *n = NODE_FROM_ELEM(l, elem);
  if (n->prev == &l->root)
    return NULL;
  return n->prev->prev->next;
}


ListStatus
list_append (List *l, void *elem)
{
  ListNode *last = l->root.prev;
  insert_between(l, last, &l->root, elem);
  return LIST_OK;
}

ListStatus
list_prepend (List *l, void *elem)
{
  ListNode *first = NODE_FROM_ELEM(l, l->root.next);
  insert_between(l, &l->root, first, elem);
  return LIST_OK;
}

ListStatus
list_append_after (List *l, void *target, void *elem)
{
  ListNode *a = NODE_FROM_ELEM(l, target);
  ListNode *b = NODE_FROM_ELEM(l, a->next);
  insert_between(l, a, b, elem);
  return LIST_OK;
}

ListStatus
list_append_before (List *l, void *target, void *elem)
{
  ListNode *b = NODE_FROM_ELEM(l, target);
  insert_between(l, b->prev, b, elem);
  return LIST_OK;
}


void*
list_remove (List *l, void *elem)
{
  ListNode *n = NODE_FROM_ELEM(l, elem);
  if (list_node_unlink (l, n) == LIST_ERR)
    return NULL;
  return elem;
}


static inline void
insert_between (List *l, ListNode *a, ListNode *b, void *elem)
{
  ListNode *n = NODE_FROM_ELEM(l, elem);
  n->next = b->prev->next;
  n->prev = a;
  a->next = elem;
  b->prev = n;
  l->size++;
}




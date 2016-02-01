#include "map.h"


#define SIZEUP(N) (((N) * 2) - 1)

#define KEY(M, E) ((void*) ((size_t) E + M->key_offset))
#define NODE(M, E) ((MapNode*) ((size_t) E + M->node_offset))
#define ELEM(M, N) ((void *) ((size_t) N - M->node_offset))
#define HASH(M, K) ((size_t) (M->hash(K, M->key_size)))

static void init_anchors (MapNode **buckets, size_t size);
static void free_anchors (MapNode **buckets, size_t size);
static MapStatus add_node (Map *m, MapNode *node);
static MapStatus maybe_rehash (Map *m);


MapStatus
map_node_init (MapNode *n)
{
  n->key = NULL;
  n->hash = 0;
  n->next = NULL;
  return MAP_OK;
}


Map*
map_new_with_offsets (size_t node_offset, size_t key_offset, size_t key_size)
{
  Map *m = malloc(sizeof(Map));
  if (!m)
    return NULL;
  m->hash = map_default_hash;
  m->cmp = map_default_comparator;
  m->buckets = calloc(MAP_INIT_SIZE, sizeof(MapNode*));
  m->node_offset = node_offset;
  m->key_offset = key_offset;
  m->key_size = key_size;
  m->nbuckets = MAP_INIT_SIZE;
  m->nelements = 0;
  init_anchors(m->buckets, m->nbuckets);
  return m;
}


Map*
string_map_new_with_offsets (size_t node_offset, size_t key_offset)
{
  Map *m = map_new_with_offsets(node_offset, key_offset, 0);
  m->hash = map_string_hash;
  m->cmp = map_string_comparator;
  return m;
}


MapStatus
map_free (Map *m)
{
  if (!m || !m->buckets)
    return MAP_ERR;
  free_anchors(m->buckets, m->nbuckets);
  free(m->buckets);
  free(m);
  return MAP_OK;
}


MapStatus
map_free_all (Map *m, void (*_free) (void*))
{
  void *last = NULL;
  MAP_FOREACH (void, e, m) {
    if (last)
      _free(last);
    last = e;
  }
  _free(last);
  return map_free(m);
}


MapStatus
map_clear (Map *m)
{
  MapNode *next = NULL;
  size_t i = m->nbuckets;
  while (i--) {
    m->buckets[i]->next = next;
    next = m->buckets[i];
  }
  m->nelements = 0;
  return MAP_OK;
}


MapStatus
map_add (Map *m, void *elem)
{
  MapNode *node = NODE(m, elem);
  node->key = KEY(m, elem);
  if (!node->key)
    return MAP_ERR;
  node->hash = HASH(m, node->key);
  if (maybe_rehash(m) == MAP_ERR)
    return MAP_ERR;
  return add_node(m, node);
}


void*
map_get (Map *m, void *key)
{
  MapNode *bucket = m->buckets[ HASH(m, key) % m->nbuckets ]->next;
  while (bucket && bucket->key) {
    if (m->cmp(key, bucket->key, m->key_size) == 0)
      return ELEM(m, bucket);
    bucket = bucket->next;
  }
  return NULL;
}


void*
map_remove (Map *m, void *key)
{
  uint32_t index = HASH(m, key) % m->nbuckets;
  MapNode *prev = m->buckets[index];
  MapNode *bucket = prev->next;
  while (bucket && bucket->key) {
    if (m->cmp(key, bucket->key, m->key_size) == 0) {
      prev->next = bucket->next;
      map_node_init(bucket);
      m->nelements--;
      return ELEM(m, bucket);
    }
    prev = bucket;
    bucket = bucket->next;
  }
  return NULL;
}


void**
map_items (Map *m)
{
  void **elems = calloc(m->nelements, sizeof(void*));
  if (!elems)
    return NULL;
  void **elem = elems;
  MapNode *n;
  for (size_t i = 0; i < m->nbuckets; i++) {
    n = m->buckets[i]->next;
    while (n && n->key) {
      *elem++ = ELEM(m, n);
      n = n->next;
    }
  }
  return elems;
}


void*
map_head (Map *m)
{
  return map_next(m, ELEM(m, m->buckets[0]));
}


void*
map_next (Map *m, void *elem)
{
  MapNode *node = NODE(m, elem);
  do
    node = node->next;
  while
    (node && !node->key);
  return node ? ELEM(m, node) : NULL;
}


static void
init_anchors (MapNode **buckets, size_t size)
{
  MapNode *next = NULL;
  while (size--) {
    buckets[size] = malloc(sizeof(MapNode));
    map_node_init(buckets[size]);
    buckets[size]->next = next;
    next = buckets[size];
  }
}


static void
free_anchors (MapNode **buckets, size_t size)
{
  while (size--)
    free(buckets[size]);
}


static MapStatus
add_node (Map *m, MapNode *node)
{
  size_t index = node->hash % m->nbuckets;
  MapNode *prev = m->buckets[index];
  MapNode *bucket = prev->next;
  while (bucket && bucket->key) {
    if (!m->cmp(node->key, bucket->key, m->key_size))
      return MAP_ERR;
    prev = bucket;
    bucket = bucket->next;
  }
  node->next = prev->next;
  prev->next = node;
  m->nelements++;
  return MAP_OK;
}


static MapStatus
maybe_rehash (Map *m)
{
  if ((float) m->nelements / m->nbuckets < MAP_HIGH_WATERMARK)
    return MAP_OK;

  size_t old_size = m->nbuckets;
  size_t new_size = SIZEUP(old_size);
  MapNode **new_buckets = calloc(new_size, sizeof(MapNode*));
  if (!new_buckets)
    return MAP_ERR;
  else
    init_anchors(new_buckets, new_size);
  MapNode **old_buckets = m->buckets;
  m->buckets = new_buckets;
  m->nbuckets = new_size;
  m->nelements = 0;

  MapNode *node, *next;
  for (size_t i = 0; i < old_size; i++) {
    node = old_buckets[i]->next;
    while (node && node->key) {
      next = node->next;
      add_node(m, node);
      node = next;
    }
  }
  free_anchors(old_buckets, old_size);
  free(old_buckets);
  return MAP_OK;
}


#define MIN(x, y) (y ^ ((x ^ y) & -(x < y)))
#define MAX(x, y) (x ^ ((x ^ y) & -(x < y)))

#define container_of(P, T, M) ((T*) ((size_t) P - offsetof(T, M)))

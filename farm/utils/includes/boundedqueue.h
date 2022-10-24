#if !defined(BOUNDED_QUEUE_H)
#define BOUNDED_QUEUE_H

#include <pthread.h>

typedef struct BQueue {
    void   **buf;
    size_t   head;
    size_t   tail;
    size_t   qsize;
    size_t   qlen;
    pthread_mutex_t  m;
    pthread_cond_t   cfull;
    pthread_cond_t   cempty;
} BQueue_t;

// it returns: q (queue pointer) -> success, NULL -> error
BQueue_t *initBQueue(size_t n);

void deleteBQueue(BQueue_t *q, void (*F)(void*));

// it returns: 0 -> success, -1 -> error
int push(BQueue_t *q, void *data);

void *pop(BQueue_t *q);

#endif /* BOUNDED_QUEUE_H */

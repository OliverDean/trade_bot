#ifndef LOCK_FREE_QUEUE_H
#define LOCK_FREE_QUEUE_H

#include <stdatomic.h>
#include <stdbool.h>


typedef struct LockFreeQueueNode {
    void *data;
    struct LockFreeQueueNode *next;
} LockFreeQueueNode;

typedef struct {
    _Atomic(struct LockFreeQueueNode *) head;
    _Atomic(struct LockFreeQueueNode *) tail;
} LockFreeQueue;

LockFreeQueue *lock_free_queue_init();
void lock_free_queue_enqueue(LockFreeQueue *queue, void *data);
void *lock_free_queue_dequeue(LockFreeQueue *queue);
void lock_free_queue_destroy(LockFreeQueue *queue);
bool queue_is_empty(LockFreeQueue *queue);

#endif // LOCK_FREE_QUEUE_H

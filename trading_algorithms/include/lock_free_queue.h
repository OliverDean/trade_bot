#ifndef LOCK_FREE_QUEUE_H
#define LOCK_FREE_QUEUE_H

#include <stdbool.h>

//typedef struct MarketData MarketData; // Forward declaration of MarketData

typedef struct LockFreeQueueNode {
    MarketData *data;
    struct LockFreeQueueNode *next;
} LockFreeQueueNode;

typedef struct LockFreeQueue {
    LockFreeQueueNode *head;
    LockFreeQueueNode *tail;
} LockFreeQueue;

LockFreeQueue *lock_free_queue_init();
bool lock_free_queue_enqueue(LockFreeQueue *queue, MarketData *data);
MarketData *lock_free_queue_dequeue(LockFreeQueue *queue);
void lock_free_queue_destroy(LockFreeQueue *queue);

#endif // LOCK_FREE_QUEUE_H

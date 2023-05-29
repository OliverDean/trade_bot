#include <stdlib.h>
#include <stdatomic.h>
#include "lock_free_queue.h"
#include "market_data_array.h"

LockFreeQueue *lock_free_queue_init() {
    LockFreeQueueNode *dummy = (LockFreeQueueNode *)malloc(sizeof(LockFreeQueueNode));
    dummy->next = NULL;

    LockFreeQueue *queue = (LockFreeQueue *)malloc(sizeof(LockFreeQueue));
    queue->head = dummy;
    queue->tail = dummy;

    return queue;
}

bool lock_free_queue_enqueue(LockFreeQueue *queue, MarketData *data) {
    LockFreeQueueNode *node = (LockFreeQueueNode *)malloc(sizeof(LockFreeQueueNode));
    node->data = data;
    node->next = NULL;

    LockFreeQueueNode *tail, *next;

    while (1) {
        tail = queue->tail;
        next = tail->next;

        if (tail == queue->tail) {
            if (next == NULL) {
                if (atomic_compare_exchange_weak(&tail->next, &next, node))
                    break;
            } else {
                atomic_compare_exchange_weak(&queue->tail, &tail, next);
            }
        }
    }

    atomic_compare_exchange_weak(&queue->tail, &tail, node);
    return true;
}

MarketData *lock_free_queue_dequeue(LockFreeQueue *queue) {
    LockFreeQueueNode *head, *tail, *next;
    MarketData *data;

    while (1) {
        head = queue->head;
        tail = queue->tail;
        next = head->next;

        if (head == queue->head) {
            if (head == tail) {
                if (next == NULL)
                    return NULL;

                atomic_compare_exchange_weak(&queue->tail, &tail, next);
            } else {
                data = next->data;
                if (atomic_compare_exchange_weak(&queue->head, &head, next))
                    break;
            }
        }
    }

    free(head);
    return data;
}

void lock_free_queue_destroy(LockFreeQueue *queue) {
    MarketData *data;
    while ((data = lock_free_queue_dequeue(queue)) != NULL)
        free(data);

    free(queue->head);
    free(queue);
}

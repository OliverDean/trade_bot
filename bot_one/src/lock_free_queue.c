#include "lock_free_queue.h"
#include <stdlib.h>

LockFreeQueue *lock_free_queue_init() {
    LockFreeQueueNode *dummy = (LockFreeQueueNode *)malloc(sizeof(LockFreeQueueNode));
    dummy->next = NULL;

    LockFreeQueue *queue = (LockFreeQueue *)malloc(sizeof(LockFreeQueue));
    atomic_store(&queue->head, dummy);
    atomic_store(&queue->tail, dummy);

    return queue;
}

void lock_free_queue_enqueue(LockFreeQueue *queue, void *data) {
    LockFreeQueueNode *node = (LockFreeQueueNode *)malloc(sizeof(LockFreeQueueNode));
    node->data = data;
    node->next = NULL;

    LockFreeQueueNode *prev_tail = atomic_exchange(&queue->tail, node);
    atomic_store(&prev_tail->next, node);
}

void *lock_free_queue_dequeue(LockFreeQueue *queue) {
    LockFreeQueueNode *head;
    void *data;

    while (1) {
        head = atomic_load(&queue->head);
        LockFreeQueueNode *next = atomic_load(&head->next);

        if (next == NULL) {
            return NULL;
        }

        if (atomic_compare_exchange_weak(&queue->head, &head, next)) {
            data = next->data;
            free(head);
            return data;
        }
    }
}

void lock_free_queue_destroy(LockFreeQueue *queue) {
    LockFreeQueueNode *node = atomic_load(&queue->head);
    while (node != NULL) {
        LockFreeQueueNode *next = node->next;
        free(node);
        node = next;
    }
    free(queue);
}

bool queue_is_empty(LockFreeQueue *queue) {
    LockFreeQueueNode *head = atomic_load(&queue->head);
    LockFreeQueueNode *tail = atomic_load(&queue->tail);
    return head == tail;
}

#include <stdlib.h>
#include <stdatomic.h>
#include "market_data_array.h"
#include "pre_processing_binance.h"
#include "proces_queue.h"

ProcesLockFreeNode *proces_queue_init() {
    ProcessQueueNode *dummy = (ProcessQueueNode *)malloc(sizeof(ProcessQueueNode));
    dummy->next = NULL;

    ProcesLockFreeNode *queue = (ProcesLockFreeNode *)malloc(sizeof(ProcesLockFreeNode));
    queue->head = dummy;
    queue->tail = dummy;

    return queue;
}

bool proces_enqueue(ProcesLockFreeNode *queue, PreProcessedData *data) {
    ProcessQueueNode *node = (ProcessQueueNode *)malloc(sizeof(ProcessQueueNode));
    node->data = data;
    node->next = NULL;

    ProcessQueueNode *tail, *next;

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

PreProcessedData *proces_dequeue(ProcesLockFreeNode *queue) {
    ProcessQueueNode *head, *tail, *next;
    PreProcessedData *data;

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

void proces_queue_destroy(ProcesLockFreeNode *queue) {
    PreProcessedData *data;
    while ((data = proces_dequeue(queue)) != NULL)
        free(data);

    free(queue->head);
    free(queue);
}

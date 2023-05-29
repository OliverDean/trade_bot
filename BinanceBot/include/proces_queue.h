#ifndef PROCES_QUEUE_H
#define PROCES_QUEUE_H

#include <stdlib.h>
#include <stdatomic.h>
#include "market_data_array.h"
#include "pre_processing_binance.h"

typedef struct ProcessQueueNode ProcessQueueNode;
typedef struct ProcesLockFreeNode ProcesLockFreeNode;

struct ProcessQueueNode {
    PreProcessedData *data;
    ProcessQueueNode *next;
};

struct ProcesLockFreeNode {
    ProcessQueueNode *head;
    ProcessQueueNode *tail;
};

ProcesLockFreeNode *proces_queue_init();
bool proces_enqueue(ProcesLockFreeNode *queue, PreProcessedData *data);
PreProcessedData *proces_dequeue(ProcesLockFreeNode *queue);
void proces_queue_destroy(ProcesLockFreeNode *queue);

#endif // PROCES_QUEUE_H

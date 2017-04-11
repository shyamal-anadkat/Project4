#ifndef QUEUE_H
#define QUEUE_H

struct Queue
{
    int front, rear, size;
    unsigned capacity;
    int* array;
};

struct Queue* initQueue(unsigned capacity);
int isFull(struct Queue* queue);
int isEmpty(struct Queue* queue);
void enqueue(struct Queue* queue, int item);
int dequeue(struct Queue* queue);

#endif
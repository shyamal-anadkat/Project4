#ifndef QUEUE_H
#define QUEUE_H

typedef struct queuenode {
	void *data;
	struct queuenode *next;
} queuenode;

typedef struct queue {
	queuenode *head;
	queuenode *tail;
	int size;
	//unsigned pending;
	lock lock;
} queue;

void queue_init(queue *queue);
int queue_push(queue *queue, void *data);
void *queue_pop(queue *queue);

#endif
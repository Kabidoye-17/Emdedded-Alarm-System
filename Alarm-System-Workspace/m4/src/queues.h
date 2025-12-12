#ifndef QUEUES_H
#define QUEUES_H

#include "queue.h"

// Command queue for ISR-to-task communication
#define MOTION_QUEUE_LENGTH 10
#define COMMAND_QUEUE_LENGTH 10
#define CLOUD_QUEUE_LENGTH 10

extern QueueHandle_t motion_queue;
extern QueueHandle_t command_queue;
extern QueueHandle_t cloud_update_queue;

void init_queues(void);

#endif /* QUEUES_H */
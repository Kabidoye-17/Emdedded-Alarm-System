#ifndef QUEUES_H
#define QUEUES_H

#include "queue.h"

#define MOTION_QUEUE_LENGTH 10
#define COMMAND_QUEUE_LENGTH 10
#define CLOUD_QUEUE_LENGTH 20 // Can get backed up if no connectivity

// motion_events sent from motion task -> handled by alert controller task, state updated as needed
extern QueueHandle_t motion_queue;
// command_events sent from cloud task -> handled by alert controller task, state updated as needed
extern QueueHandle_t command_queue;
// cloud_update_events sent from alert controller task based on any state update -> handled by cloud task
extern QueueHandle_t cloud_update_queue;

// Initialize queues to corresponding lengths
void init_queues(void);

#endif /* QUEUES_H */
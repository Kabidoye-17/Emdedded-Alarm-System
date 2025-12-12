#include "queues.h"
#include "typing.h"
#include "FreeRTOS.h"

// Initialize queues
void init_queues(void) {
    motion_queue = xQueueCreate(MOTION_QUEUE_LENGTH, sizeof(motion_event));
    command_queue = xQueueCreate(COMMAND_QUEUE_LENGTH, sizeof(command_event));
    cloud_update_queue = xQueueCreate(CLOUD_QUEUE_LENGTH, sizeof(cloud_update_event));
}
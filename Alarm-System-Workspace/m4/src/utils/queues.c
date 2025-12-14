#include <stdint.h>

#include "FreeRTOS.h"
#include "queues.h"
#include "typing.h"

// Define queue handles (matching the extern declarations in queues.h)
QueueHandle_t motion_queue = NULL;
QueueHandle_t command_queue = NULL;
QueueHandle_t cloud_update_queue = NULL;

// Initialize queues
void init_queues(void) {
    motion_queue = xQueueCreate(MOTION_QUEUE_LENGTH, sizeof(motion_event));
    command_queue = xQueueCreate(COMMAND_QUEUE_LENGTH, sizeof(command_event));
    cloud_update_queue = xQueueCreate(CLOUD_QUEUE_LENGTH, sizeof(cloud_update_event));
}
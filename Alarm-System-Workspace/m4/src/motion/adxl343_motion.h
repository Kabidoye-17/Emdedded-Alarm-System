#ifndef ADXL343_MOTION_H
#define ADXL343_MOTION_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"


/* Initializes sensor + GPIO + starts motion task */
int adxl343_motion_start(QueueHandle_t motionQueue,
                         UBaseType_t priority,
                         uint16_t stackDepth);


#endif

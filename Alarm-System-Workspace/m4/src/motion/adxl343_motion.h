#ifndef ADXL343_MOTION_H
#define ADXL343_MOTION_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"


/***** Motion detection task *****/
/*
 * This task waits for motion interrupts signaled by the ISR.
 * It prioritizes events and sends the highest-priority warning
 * to the motion queue.
 */
void MotionDetectionTask(void *arg);

#endif

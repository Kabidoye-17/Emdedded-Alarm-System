#include "task_handler.h"

#include "FreeRTOS.h"
#include "task.h"

#include "../alarm/alert_outputs.h"
#include "../alarm/alert_control.h"
#include "watchdog.h"
#include "../motion/adxl343_motion.h"
#include "../uart/cloud_tasks.h"

/*
 * Priorities explained (Low to High):

 * - LED Effects Task: Low priority (1) as it is purely cosmetic and non-critical. 
 *    It only reflects system state set by higher-priority tasks, so delaying LED updates does not affect functionality.
 *    CPU time should be given to critical tasks first.
 * 
 * - Alert Control Task: Medium priority (tskIDLE_PRIORITY + 1) processes alert events from motion/watchdog
 *    and manages the alert state machine. Must run before Cloud Send to ensure alerts are processed before
 *    transmission, but lower than Motion Detection since data capture takes priority.
 * 
 * - Watchdog Task: Medium priority (tskIDLE_PRIORITY + 1) performs periodic health monitoring and timeout
 *    checks. Needs reasonable responsiveness but does not require real-time constraints like motion detection.
 * 
 * - Cloud Send Task: Medium priority (tskIDLE_PRIORITY + 1) transmits data over UART/MQTT. Medium priority
 *    prevents blocking while maintaining timely communication. Lower than Motion Detection to prioritize
 *    real-time sensor data capture.
 * 
 * - Motion Detection Task: High priority (configMAX_PRIORITIES - 1) captures accelerometer data in real-time.
 *    Time-critical sensor sampling cannot be delayed without losing motion events. Highest priority ensures
 *    consistent sampling rates and prevents motion data loss from preemption by other tasks.
 * 
 * 
 * Stack sizes explained (conservative estimates to prevent overflow):
 * 
 * - LED Effects Task: 512 bytes handles LED pattern state, array indexing, and simple control flow without
 *    deep function call stacks. Sufficient for non-recursive LED state management.
 * 
 * - Alert Control Task: 1024 bytes supports state machine logic, queue operations, and multiple alert
 *    condition checks. Largest stack due to handling multiple queues (motion alerts, watchdog events) and
 *    branching alert logic that may nest function calls.
 * 
 * - Motion Detection Task: 512 bytes for sensor data processing (ADXL343 calculations, filtering algorithms,
 *    and event detection) with moderate function call depth. Accelerometer data structures and math operations
 *    are bounded and do not require excessive recursion.
 * 
 * - Watchdog Task: 256 bytes minimal stack for simple periodic timer checks and system health flags. Task
 *    performs only basic comparisons and register updates without complex logic.
 * 
 * - Cloud Send Task: 256 bytes sufficient for UART frame construction and transmission. Minimal processing
 *    since data is already formatted by Alert Control Task. Simple send-and-wait operations do not require
 *    large local buffers or deep call stacks.
*/

void create_LED_control_task(void) {
    xTaskCreate(LedEffectTask, "LEDEffects", 512, NULL, tskIDLE_PRIORITY + 1,NULL);
}

void create_alert_control_task(void) {
    xTaskCreate(AlertControlTask, "AlertControl", 1024, NULL, tskIDLE_PRIORITY + 1, NULL);
}

void create_watchdog_task(void) {
    xTaskCreate(WatchdogTask, "WDT", 256, NULL, tskIDLE_PRIORITY + 1, NULL );

}

void create_motion_detection_task(void) {
    xTaskCreate(MotionDetectionTask, "MotionDetect", 512, NULL, configMAX_PRIORITIES - 1, NULL);
}

void create_cloud_send_task(void) {
    xTaskCreate(cloud_send_task, "CloudSend", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
}

void create_all_tasks(void) {
    create_LED_control_task();
    create_alert_control_task();
    create_motion_detection_task();
    create_watchdog_task();
    create_cloud_send_task();
}
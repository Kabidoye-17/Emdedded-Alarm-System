#include "task_handler.h"

#include "FreeRTOS.h"
#include "task.h"

#include "../alarm/alert_outputs.h"
#include "../alarm/alert_control.h"
#include "watchdog.h"
#include "../motion/adxl343_motion.h"
#include "../uart/cloud_tasks.h"

void create_LED_control_task(void) {
    xTaskCreate(LedEffectTask, "LEDEffects", 512, NULL, 1,NULL);
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
    xTaskCreate( cloud_send_task, "CloudSend", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
}

void create_all_tasks(void) {
    create_LED_control_task();
    create_alert_control_task();
    create_motion_detection_task();
    create_watchdog_task();
    create_cloud_send_task();
}
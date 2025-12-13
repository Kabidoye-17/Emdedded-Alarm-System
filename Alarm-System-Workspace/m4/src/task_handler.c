#include "task_handler.h"

#include "FreeRTOS.h"
#include "task.h"

#include "alarm/alert_outputs.h"
#include "alert_control.h"

void create_LED_control_task(void) {
    xTaskCreate(LedEffectTask, "LEDEffects", 512, NULL, 1,NULL);
}

void create_alert_control_task(void) {
    xTaskCreate(AlertControlTask, "AlertControl", 1024, NULL, tskIDLE_PRIORITY + 1, NULL);
}

void create_all_tasks(void) {
    create_LED_control_task();
    create_alert_control_task();
}
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "alarm/state_machine.h"
#include "alarm/alert_outputs.h"
#include "alarm/led_driver.h"
#include "queues.h"

QueueSetHandle_t alert_queue_set;
#define SET_LENGTH (MOTION_QUEUE_LENGTH + COMMAND_QUEUE_LENGTH)

static TimerHandle_t warn_timeout_timer;


// Drive the physical alerts for the current state. 
static void apply_alerts(alarm_state state) {
	switch (state) {
	case DISARMED:
		alert_outputs_set_mode(BLUE);
		break;
	case ARMED_IDLE:
		alert_outputs_set_mode(GREEN);
		break;
	case WARN:
        alert_outputs_set_mode(RED);
		break;
    case ALERT:
		alert_outputs_set_mode(RED_BREATHE);
		break;
	case ALARM:
        alert_outputs_set_mode(RED_FLASH);
		break;
	default:
		break;
	}
}

static alarm_event warn_to_alarm_event(warn_type warning) {
    switch (warning) {
        case LOW_WARN:
            return EVENT_LOW_WARN;
        case MED_WARN:
            return EVENT_MED_WARN;
        case HIGH_WARN:
            return EVENT_HIGH_WARN;
        default:
            return EVENT_LOW_WARN;
    }
}

static alarm_event command_to_alarm_event(command_type cmd) {
    switch (cmd) {
        case ARM:
            return EVENT_ARM_SYSTEM;
        case DISARM:
            return EVENT_DISARM_SYSTEM;
        case RESOLVE_ALARM:
            return EVENT_RESOLVE_ALARM;
        case CANCEL_WARN:
            return EVENT_CANCEL_WARN;
        default:
            return EVENT_ARM_SYSTEM;
    }
}

int send_cloud_update(cloud_update_event* update) {
    // Try to send with timeout
    if (xQueueSend(cloud_update_queue, update, pdMS_TO_TICKS(100)) != pdPASS) {
        // Queue full - drop oldest message to make room
        cloud_update_event discarded;
        xQueueReceive(cloud_update_queue, &discarded, 0);

        // Retry sending new update (should succeed now)
        xQueueSend(cloud_update_queue, update, 0);
    }
    return pdPASS;
}

static void warn_timeout_callback(TimerHandle_t xTimer) {
    command_event cancel_cmd = {.cmd = CANCEL_WARN};
    xQueueSend(command_queue, &cancel_cmd, 0);
}

// Only this task touches LEDs
void AlertControlTask(void *arg){
    static alarm_sm alarm_machine;
    alarm_sm_init(&alarm_machine);
    init_GPIO_for_LEDs();

    alert_queue_set = xQueueCreateSet(SET_LENGTH);

    xQueueAddToSet(motion_queue, alert_queue_set);
    xQueueAddToSet(command_queue, alert_queue_set);
    
    warn_timeout_timer = xTimerCreate("WarnTimeout",
                                      pdMS_TO_TICKS(5000), // 5 second timeout
                                      pdFALSE,
                                      NULL,
                                      warn_timeout_callback);
    
    while (1) {
        QueueSetMemberHandle_t activated_queue;

        // Take whichever queue has an event
        activated_queue = xQueueSelectFromSet(alert_queue_set, portMAX_DELAY);
        
        // Process the queue based on which one was activated (ready to read)
        if (activated_queue == motion_queue) {
            motion_event m_e;
            xQueueReceive(motion_queue, &m_e, 0);
            alarm_event new_event = warn_to_alarm_event(m_e.warning);
            alarm_state old_state = alarm_sm_state(&alarm_machine);
            alarm_sm_handle_event(&alarm_machine, new_event);
            alarm_state new_state = alarm_sm_state(&alarm_machine);
            if (new_state != old_state) {
                apply_alerts(alarm_machine.state);
                // Start warn timeout if entering WARN state
                if (new_state == WARN) {
                    xTimerStart(warn_timeout_timer, 0);
                }
                // Stop warn timeout if leaving WARN state
                if (old_state == WARN && new_state != WARN) {
                    xTimerStop(warn_timeout_timer, 0);
                }
                cloud_update_event update = {0};
                update.from_motion = 1;
                update.warning = m_e.warning;
                update.state = new_state;
                send_cloud_update(&update);
            }
        }

        if (activated_queue == command_queue) {
            command_event c_e;
            xQueueReceive(command_queue, &c_e, 0);
            alarm_event new_event = command_to_alarm_event(c_e.cmd);
            alarm_state old_state = alarm_sm_state(&alarm_machine);
            alarm_sm_handle_event(&alarm_machine, new_event);
            alarm_state new_state = alarm_sm_state(&alarm_machine);
            if (new_state != old_state) {
                apply_alerts(alarm_machine.state);
                // Stop warn timeout if leaving WARN state
                if (old_state == WARN && new_state != WARN) {
                    xTimerStop(warn_timeout_timer, 0);
                }
                cloud_update_event update = {0};
                update.from_motion = 0;
                update.state = new_state;
                send_cloud_update(&update);
            }
        }
    }
}
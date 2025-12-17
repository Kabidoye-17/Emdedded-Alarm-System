#ifndef TASK_HANDLER_H
#define TASK_HANDLER_H

/*
 * Abstraction layer for creating and managing FreeRTOS tasks.
 * Tasks are created with appropriate stack sizes and priorities.
 * Allows for centralized task management.
 * Ensures modularity and easier maintenance.
*/

void create_LED_control_task(void);
void create_alert_control_task(void);
void create_watchdog_task(void);
void create_motion_detection_task(void);
void create_cloud_send_task(void);

void create_all_tasks(void);


#endif /* TASK_HANDLER_H */
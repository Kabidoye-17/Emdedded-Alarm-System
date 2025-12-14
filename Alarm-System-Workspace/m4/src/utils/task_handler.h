#ifndef TASK_HANDLER_H
#define TASK_HANDLER_H


void create_LED_control_task(void);
void create_alert_control_task(void);
void create_watchdog_task(void);
void create_cloud_send_task(void);
void create_all_tasks(void);


#endif /* TASK_HANDLER_H */
#ifndef ALERT_CONTROL_H
#define ALERT_CONTROL_H

#include "../utils/typing.h"

// -> add cloud update event to cloud queue to be processed
int send_cloud_update(cloud_update_event* update);

/* Alert Control Task:
-> Monitors state machine
-> Processes events from motion and command queues
-> Applies physical alerts
-> Sends cloud update events
*/
void AlertControlTask(void *arg);

#endif /* ALERT_CONTROL_H */
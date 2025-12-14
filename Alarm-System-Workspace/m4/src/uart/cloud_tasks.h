#ifndef CLOUD_TASKS_H
#define CLOUD_TASKS_H

#include "../utils/typing.h"

/**
 * @brief UART RX callback - sends command to queue from ISR context
 *
 * Called by UART ISR when valid command frame received.
 * Sends command_event to command_queue with ISR-safe queue operations.
 * Implements drop-oldest strategy if queue full.
 *
 * @param cmd command_type enum from UART parser
 */
void on_message_received(command_type cmd);

/**
 * @brief Cloud send task - consumes cloud_update_queue and transmits via UART
 *
 * Monitors cloud_update_queue for events from AlertControlTask, serializes
 * to pipe-delimited format, and transmits via UART with timeout/retry logic.
 * Implements connection status tracking (online/offline).
 *
 * @param pvParameters FreeRTOS task parameter (unused)
 */
void cloud_send_task(void *pvParameters);

#endif /* CLOUD_TASKS_H */

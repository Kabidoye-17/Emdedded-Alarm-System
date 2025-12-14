/**
 *
 * Copyright (c) 2025 Analog Devices, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cloud_tasks.h"
#include "typing.h"
#include "queues.h"
#include "uart_coms.h"
#include "board.h"

// Configuration constants
#define TX_TIMEOUT_MS 100
#define OFFLINE_THRESHOLD 3
#define INTER_MESSAGE_DELAY_MS 50
#define RETRY_BACKOFF_MS 500

/**
 * @brief UART RX callback - sends command to queue from ISR context
 */
void on_message_received(command_type cmd) {
    command_event event;
    event.cmd = cmd;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Attempt to send to queue
    if (xQueueSendFromISR(command_queue, &event, &xHigherPriorityTaskWoken) != pdPASS) {
        // Queue full - drop oldest command to make room
        command_event discarded_event;
        xQueueReceiveFromISR(command_queue, &discarded_event, &xHigherPriorityTaskWoken);

        // Retry sending new command (should succeed now)
        xQueueSendFromISR(command_queue, &event, &xHigherPriorityTaskWoken);
    }

    // Yield to higher priority task if necessary
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief Convert warn_type enum to abbreviated string
 */
static const char* warn_type_to_string(warn_type warning) {
    switch (warning) {
        case LOW_WARN:  return "LOW";
        case MED_WARN:  return "MED";
        case HIGH_WARN: return "HIGH";
        default:        return "UNK";
    }
}

/**
 * @brief Convert alarm_state enum to string
 */
static const char* alarm_state_to_string(alarm_state state) {
    switch (state) {
        case DISARMED:    return "DISARMED";
        case ARMED_IDLE:  return "ARMED";
        case WARN:        return "WARN";
        case ALERT:       return "ALERT";
        case ALARM:       return "ALARM";
        default:          return "UNKNOWN";
    }
}

/**
 * @brief Serialize cloud_update_event to pipe-delimited format
 *
 * Format: FROM_MOTION|WARN_TYPE|ALARM_STATE
 * - Motion event: "1|HIGH|WARN"
 * - Command event: "0||DISARMED" (warn_type empty)
 *
 * @param update Pointer to cloud_update_event
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return Number of bytes written (excluding null terminator), or -1 on error
 */
static int serialize_cloud_update(const cloud_update_event* update,
                                  char* buffer,
                                  size_t buffer_size) {
    int len;

    if (update->from_motion) {
        // Motion event - include warn_type
        len = snprintf(buffer, buffer_size,
                       "%u|%s|%s",
                       update->from_motion,
                       warn_type_to_string(update->warning),
                       alarm_state_to_string(update->state));
    } else {
        // Command event - warn_type is null (empty field)
        len = snprintf(buffer, buffer_size,
                       "%u||%s",
                       update->from_motion,
                       alarm_state_to_string(update->state));
    }

    if (len < 0 || len >= (int)buffer_size) {
        return -1;  // Error or buffer overflow
    }

    return len;
}

/**
 * @brief Cloud send task - processes cloud_update_queue and handles transmission
 *
 * Dequeues cloud update events and transmits them via UART with timeout detection.
 * Implements retry logic and connection status tracking:
 * - ONLINE: Queue drains immediately, messages sent in real-time
 * - OFFLINE: Retries failed messages, accumulates queue
 * - RECOVERY: Drains queue when gateway returns online
 *
 */
void cloud_send_task(void *pvParameters) {
    cloud_update_event update;
    char buffer[16 + 1];  // MAX_DATA_LENGTH + null terminator
    uint8_t failure_count = 0;
    bool is_online = true;

    while (1) {
        // Wait for cloud update with timeout
        if (xQueueReceive(cloud_update_queue, &update, pdMS_TO_TICKS(100)) == pdPASS) {

            // Serialize to pipe-delimited format
            int len = serialize_cloud_update(&update, buffer, sizeof(buffer));

            if (len < 0) {
                continue;  // Discard and move to next message
            }

            // Attempt transmission with timeout
            int result = uart_send_frame_with_timeout(
                (const uint8_t*)buffer,
                (uint8_t)len,
                TX_TIMEOUT_MS
            );

            if (result == 0) {
                // SUCCESS - transmission completed
                failure_count = 0;

                // Check if recovering from offline state
                if (!is_online) {
                           (int)uxQueueMessagesWaiting(cloud_update_queue);
                    is_online = true;
                }

                // Inter-message delay to avoid overwhelming gateway during queue drain
                vTaskDelay(pdMS_TO_TICKS(INTER_MESSAGE_DELAY_MS));

            } else {
                // FAILURE - transmission timed out
                failure_count++;

                // Check if gateway went offline
                if (failure_count >= OFFLINE_THRESHOLD && is_online) {
                    is_online = false;
                }

                // Re-queue message at front for retry (preserves FIFO order)
                xQueueSendToFront(cloud_update_queue, &update, 0);

                // Backoff delay before retry to avoid busy-wait
                vTaskDelay(pdMS_TO_TICKS(RETRY_BACKOFF_MS));
            }
        }
    }
}

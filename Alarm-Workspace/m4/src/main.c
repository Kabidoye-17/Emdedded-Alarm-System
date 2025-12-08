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
#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "mxc_device.h"
#include "led.h"
#include "pb.h"
#include "board.h"
#include "uart_coms.h"
#include "json_helper.h"

// Shared data protected by semaphore
static command_event latest_event;
static SemaphoreHandle_t event_semaphore;

// UART callback - gives semaphore on successful parse
void on_message_received(const char* json_str) {
    if (parse_command_event(json_str, &latest_event) == 0) {
        xSemaphoreGiveFromISR(event_semaphore, NULL);
    }
}

// Task: Process commands
void command_task(void *pvParameters) {
    while (1) {
        // Wait for semaphore from UART callback
        if (xSemaphoreTake(event_semaphore, portMAX_DELAY) == pdTRUE) {
            // Flash LED based on command type
            switch (latest_event.cmd) {
                case ARM:
                    LED_On(LED_RED);
                    vTaskDelay(pdMS_TO_TICKS(200));
                    LED_Off(LED_RED);
                    break;
                case DISARM:
                    LED_On(LED_GREEN);
                    vTaskDelay(pdMS_TO_TICKS(200));
                    LED_Off(LED_GREEN);
                    break;
                case RESOLVE:
                    LED_On(LED_BLUE);
                    vTaskDelay(pdMS_TO_TICKS(200));
                    LED_Off(LED_BLUE);
                    break;
                default:
                    break;
            }
        }
    }
}

int main(void) {
    printf("Alarm System Starting...\n");

    // Create binary semaphore for signaling
    event_semaphore = xSemaphoreCreateBinary();

    // Initialize UART
    uart_init(on_message_received);

    // Create command task
    xTaskCreate(
        command_task,
        "Command",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        2,
        NULL
    );

    // Start scheduler
    vTaskStartScheduler();
    
    return 0;
}

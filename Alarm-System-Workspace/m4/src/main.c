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
#include "queue.h"
#include "mxc_device.h"
#include "led.h"
#include "pb.h"
#include "board.h"
#include "uart_coms.h"
#include "adxl343.h"
#include "mxc_delay.h"
#include "spi.h"
#include "mxc_pins.h"
#include "gpio.h"
#include "adxl343_motion.h"
#include "watchdog.h"
#include "watchdog.h"
#include "wdt.h"
#include "queues.h"
#include "task_handler.h"


/***** SPI pins *****/
#define MOSI_PIN 21
#define MISO_PIN 22
#define FTHR_Defined 1

QueueHandle_t motionQueue;

// Command queue for ISR-to-task communication
#define COMMAND_QUEUE_LENGTH 10
static QueueHandle_t command_queue = NULL;

// UART callback - sends command to queue from ISR context
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

// Task: LED debug task - flashes blue LED to indicate board is running
void led_debug_task(void *pvParameters) {
    while (1) {
        LED_Toggle(LED_BLUE);
        vTaskDelay(pdMS_TO_TICKS(500));  // Flash every 500ms
    }
}

// Task: Placeholder (your friend will add consumer logic for state machine)
void command_task(void *pvParameters) {
    while (1) {
        // Placeholder - no consumption happening
        // This allows the queue to fill up for testing overflow behavior
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void) {
    init_queues();
    create_all_tasks();

    if (MXC_WDT_GetResetFlag(MXC_WDT0)) {
        MXC_WDT_ClearResetFlag(MXC_WDT0);
    }

    setvbuf(stdout, NULL, _IONBF, 0);

    int retVal;
    mxc_spi_pins_t spi_pins;
    int selected_ss = -1;

    spi_pins.clock = TRUE;
    spi_pins.miso  = TRUE;
    spi_pins.mosi  = TRUE;
    spi_pins.sdio2 = FALSE;
    spi_pins.sdio3 = FALSE;
    spi_pins.ss0   = TRUE;
    spi_pins.ss1   = TRUE;
    spi_pins.ss2   = FALSE;

    retVal = adxl343_spi_init(&spi_pins);
    if (retVal != E_NO_ERROR)
        return retVal;

    const int ss_candidates[] = {1, 0};
    for (unsigned i = 0; i < 2; i++) {
        adxl343_set_ss(ss_candidates[i]);
        if (adxl343_probe() == E_NO_ERROR) {
            selected_ss = ss_candidates[i];
            break;
        }
    }

    if (selected_ss < 0)
        return -1;

    retVal = adxl343_init();
    if (retVal != E_NO_ERROR)
        return retVal;


    motionQueue = xQueueCreate(8, sizeof(MotionEvent));


    adxl343_motion_start(
        motionQueue,
        configMAX_PRIORITIES - 1,
        512
    );


    printf("Alarm System Starting...\n");

    // Create command queue with depth 10
    command_queue = xQueueCreate(COMMAND_QUEUE_LENGTH, sizeof(command_event));

    // Error handling - critical system component
    if (command_queue == NULL) {
        printf("FATAL: Failed to create command queue\n");
        while(1);  // Halt system
    }

    // Initialize UART
    uart_init(on_message_received);

    // Create LED debug task (low priority, just for visual confirmation)
    xTaskCreate(
        led_debug_task,
        "LED_Debug",
        configMINIMAL_STACK_SIZE,
        NULL,
        1,
        NULL
    );

    // Create command task
    xTaskCreate(
        command_task,
        "Command",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        2,
        NULL
    );

     watchdog_init();

    xTaskCreate(
        WatchdogTask,
        "WDT",
        256,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );


    vTaskStartScheduler();
    while (1);
}
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
#include "board.h"
#include "uart_coms.h"
#include "adxl343.h"
#include "spi.h"
#include "mxc_pins.h"
#include "gpio.h"
#include "adxl343_motion.h"
#include "watchdog.h"
#include "wdt.h"
#include "queues.h"
#include "task_handler.h"


/***** SPI pins *****/
#define MOSI_PIN 21
#define MISO_PIN 22


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



int main(void) {
    init_queues();


    if (MXC_WDT_GetResetFlag(MXC_WDT0)) {
        MXC_WDT_ClearResetFlag(MXC_WDT0);
    }

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


    // Initialize UART
    uart_init(on_message_received);

    watchdog_init();
    create_all_tasks();

    vTaskStartScheduler();
    while (1);
}
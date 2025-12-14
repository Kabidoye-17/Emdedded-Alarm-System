#include "watchdog.h"
#include "wdt.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

/*
 * ============================================================================
 * Watchdog Timer Module
 * ============================================================================
 * This module configures and services the hardware watchdog timer.
 *
 * Purpose:
 *  - Detect system lockups or task starvation
 *  - Automatically reset the device if the system becomes unresponsive
 *
 * The watchdog is serviced (kicked) periodically by a dedicated FreeRTOS task.
 */

/* Keep watchdog configuration static */
/*
 * Declared static so it persists for the lifetime of the program
 * and is not visible outside this file.
 */
static mxc_wdt_cfg_t cfg;


/***** Watchdog initialization *****/
/*
 * Configures the watchdog timer but does NOT enable it yet.
 * This allows the system to finish booting before the watchdog
 * becomes active.
 */
void watchdog_init(void)
{
    // Use windowed watchdog mode
    // Reset must occur between lower and upper time windows
    cfg.mode = MXC_WDT_WINDOWED;

    /*
     * Configure watchdog timing window:
     *
     * lowerResetPeriod -> minimum time before reset is allowed
     * upperResetPeriod -> maximum time before reset must occur
     *
     * This configuration prevents both:
     *  - kicking too early (lower window violation)
     *  - kicking too late (upper window violation)
     */
    cfg.lowerResetPeriod = MXC_WDT_PERIOD_2_23;  // ~1 ms
    cfg.upperResetPeriod = MXC_WDT_PERIOD_2_30;  // ~32 s

    // Initialize watchdog hardware with configuration
    MXC_WDT_Init(MXC_WDT0, &cfg);

    // Apply reset period settings
    MXC_WDT_SetResetPeriod(MXC_WDT0, &cfg);

    // Keep watchdog disabled during system startup
    MXC_WDT_Disable(MXC_WDT0);

    // Clear watchdog counter to a known state
    MXC_WDT_ResetTimer(MXC_WDT0);
}


/***** Watchdog servicing task *****/
/*
 * This FreeRTOS task is responsible for periodically resetting
 * (kicking) the watchdog timer.
 *
 * If this task stops running or is blocked indefinitely,
 * the watchdog will expire and reset the system.
 */
void WatchdogTask(void *pvParameters)
{
    (void)pvParameters;

    /*
     * Delay at startup to allow:
     *  - system initialization to complete
     *  - lower watchdog window to expire
     */
    vTaskDelay(pdMS_TO_TICKS(100));

    // Perform initial watchdog reset inside valid window
    MXC_WDT_ResetTimer(MXC_WDT0);

    // Enable watchdog-triggered system reset
    MXC_WDT_EnableReset(MXC_WDT0);

    // Enable watchdog timer
    MXC_WDT_Enable(MXC_WDT0);

    for (;;)
    {
        /*
         * Periodically reset the watchdog.
         * This delay must remain within the configured
         * watchdog window limits.
         */
        vTaskDelay(pdMS_TO_TICKS(5000));

        // Kick watchdog to prevent system reset
        MXC_WDT_ResetTimer(MXC_WDT0);
    }
}

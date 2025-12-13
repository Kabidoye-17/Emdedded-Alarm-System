#include "watchdog.h"
#include "wdt.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

/* Keep config static */
static mxc_wdt_cfg_t cfg;

void watchdog_init(void)
{
    cfg.mode = MXC_WDT_WINDOWED;

    /* Make lower window tiny */
    cfg.lowerResetPeriod = MXC_WDT_PERIOD_2_23;  // ~1 ms
    cfg.upperResetPeriod = MXC_WDT_PERIOD_2_30;  // ~32 s

    MXC_WDT_Init(MXC_WDT0, &cfg);
    MXC_WDT_SetResetPeriod(MXC_WDT0, &cfg);

    MXC_WDT_Disable(MXC_WDT0);
    MXC_WDT_ResetTimer(MXC_WDT0);

}


static void watchdog_enable_and_start(void)
{
    MXC_WDT_ResetTimer(MXC_WDT0);
    MXC_WDT_EnableReset(MXC_WDT0);
    MXC_WDT_Enable(MXC_WDT0);
}

void watchdog_kick(void)
{
    MXC_WDT_ResetTimer(MXC_WDT0);
}


void WatchdogTask(void *pvParameters)
{
    /* Let system settle and clear lower window */
    vTaskDelay(pdMS_TO_TICKS(100));

    MXC_WDT_ResetTimer(MXC_WDT0);
    MXC_WDT_EnableReset(MXC_WDT0);
    MXC_WDT_Enable(MXC_WDT0);

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
        MXC_WDT_ResetTimer(MXC_WDT0);
    }
}



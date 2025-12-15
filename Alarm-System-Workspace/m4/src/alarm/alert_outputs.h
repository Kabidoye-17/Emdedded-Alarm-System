#ifndef ALERT_OUTPUTS_H
#define ALERT_OUTPUTS_H

// Alert reaction control outputs (LEDs, buzzers, etc.)

typedef enum {
    OFF = 0,
    BLUE,
    GREEN,
    RED,
    RED_BREATHE,
    RED_FLASH
} LedMode;

// Set the LED mode for alert outputs
void alert_outputs_set_mode(LedMode mode);
// LED effect controller task to be run as FreeRTOS task
void LedEffectTask(void *arg);
#endif /* ALERT_OUTPUTS_H */
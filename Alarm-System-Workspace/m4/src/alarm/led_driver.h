#ifndef LED_DRIVER_H
#define LED_DRIVER_H

/*
 * Alert control for the alarm system.
 * Manages visual and auditory alerts based on the alarm state.
 * Always at least one alert should be active based on the current state.
 * 
 * The alert priorities are as follows (highest to lowest):
 * 1. ALARM_STATE_ALARM: Red LED flashing + siren on
 * 2. ALARM_STATE_ALERT: Red LED breathing
 * 3. ALARM_STATE_WARN: Solid red LED
 * 4. ALARM_STATE_ARMED_IDLE: Solid blue LED
 * 5. ALARM_STATE_DISARMED: Solid green LED
 * 
 * No two alerts can be active at the same time (state machine).
 * Higher priority alerts override lower priority ones.
 */

void init_GPIO_for_LEDs(void);

void turn_on_green_LED(void);

void turn_off_green_LED(void);

void turn_on_blue_LED(void);

void turn_off_blue_LED(void);

void turn_on_red_LED(void);

void turn_off_red_LED(void);

void set_red_LED_brightness(uint8_t brightness);

void stop_LED_effects(void);

#endif /* LED_DRIVER_H */
#ifndef ALERT_CONTROL_H
#define ALERT_CONTROL_H

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


// Initialize alert control system
void alert_control_init();

// Make the LED show a solid color
void turn_on_green_LED();
void turn_on_blue_LED();

// Make the yellow LED breathe until stopped
void yellow_breathe_LED();

// Make the red LED breathe until stopped
void red_LED_breathe();

// Make the red LED flash until stopped
void red_LED_flash();

// Stop any ongoing LED effects
void stop_LED_effects();


#endif /* ALERT_CONTROL_H */
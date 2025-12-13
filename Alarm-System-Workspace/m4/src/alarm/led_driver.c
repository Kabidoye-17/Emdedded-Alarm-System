#include <stdint.h>
#include "alert_outputs.h"
#include "led.h"
#include "board.h"

// Store current duty cycle (0-100%)
static volatile uint8_t red_duty_percent = 0;

// Map our semantic colors to the board LED indices.
static unsigned int red_idx = LED_RED;
static unsigned int green_idx = LED_GREEN;
static unsigned int blue_idx = LED_BLUE;

void init_GPIO_for_LEDs(void) {
    LED_Init();

    LED_Off(red_idx);
    LED_Off(green_idx);
    LED_Off(blue_idx);
}

void turn_on_green_LED(void) {
    LED_On(green_idx);
}

void turn_off_green_LED(void) {
    LED_Off(green_idx);
}

void turn_on_blue_LED(void) {
    LED_On(blue_idx);
}

void turn_off_blue_LED(void) {
    LED_Off(blue_idx);
}

void turn_on_red_LED(void) {
    LED_On(red_idx);
}

void turn_off_red_LED(void) {
    LED_Off(red_idx);
}

void set_red_LED_brightness(uint8_t duty_percent) {
    if (duty_percent > 100) duty_percent = 100;
    red_duty_percent = duty_percent;

    // Simple on/off based on 100-step software PWM
    static uint8_t pwm_counter = 0;
    pwm_counter = (pwm_counter + 1) % 100;

    if (pwm_counter < red_duty_percent) {
        turn_on_red_LED();
    } else {
        turn_off_red_LED();
    }
}

void stop_LED_effects(void) {
    turn_off_blue_LED();
    turn_off_green_LED();
    turn_off_red_LED();
}
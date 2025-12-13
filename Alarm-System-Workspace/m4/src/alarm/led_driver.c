#include "alert_outputs.h"
#include "mxc_device.h"
#include "led.h"
#include "pb.h"
#include "board.h"
#include "mxc_delay.h"
#include "gpio.h"

#define GPIO_BASE 0x40008000
#define GPIO_OUT_OFFSET 0x0018

#define GPIO_LED_PIN_BLUE 26
#define GPIO_LED_PIN_GREEN 19
#define GPIO_LED_PIN_RED 18

#define GPIO_OUT_REG (*(volatile uint32_t*)(GPIO_BASE + GPIO_OUT_OFFSET))

void turn_on_green_LED() {
    GPIO_OUT_REG |= (0x1 << GPIO_LED_PIN_GREEN);
}

void turn_off_green_LED() {
    GPIO_OUT_REG &= ~(0x1 << GPIO_LED_PIN_GREEN);
}

void turn_on_blue_LED() {
    GPIO_OUT_REG |= (0x1 << GPIO_LED_PIN_BLUE);
}

void turn_off_blue_LED() {
    GPIO_OUT_REG &= ~(0x1 << GPIO_LED_PIN_BLUE);
}

void turn_on_red_LED() {
    GPIO_OUT_REG |= (0x1 << GPIO_LED_PIN_RED);
}

void turn_off_red_LED() {
    GPIO_OUT_REG &= ~(0x1 << GPIO_LED_PIN_RED);
}

void set_red_LED_brightness(uint8_t duty_percent) {
    const int period_us = 2000;  // 2ms → 500 Hz PWM

    if (duty_percent > 100)
        duty_percent = 100;

    // Compute on/off times for this cycle
    int on_time  = (period_us * duty_percent) / 100;
    int off_time = period_us - on_time;

    // **Single PWM pulse** (one cycle only)
    if (on_time > 0) {
        turn_on_red_LED();
        MXC_Delay(on_time);
    }

    if (off_time > 0) {
        turn_off_red_LED();
        MXC_Delay(off_time);
    }
}

void stop_LED_effects() {
    turn_off_blue_LED();
    turn_off_green_LED();
    turn_off_red_LED();
}
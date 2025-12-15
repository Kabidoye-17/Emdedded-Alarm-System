#include "FreeRTOS.h"
#include "task.h"
#include "gpio.h"
#include "led_driver.h"
#include "alert_outputs.h"

static volatile LedMode current_mode = OFF;

static uint8_t breathe_step = 0;
static uint8_t flash_state = 0;

// Precomputed breathing curve (0-100 brightness)
static const uint8_t breathe_curve[] = {
    0, 5, 10, 20, 40, 60, 80, 100, 80, 60, 40, 20, 10, 5
};
static const int breathe_steps = sizeof(breathe_curve) / sizeof(breathe_curve[0]);

void alert_outputs_set_mode(LedMode mode) {
    // Prevent concurrent access while updating shared state (Race conditions)
    taskENTER_CRITICAL();
    current_mode = mode;
    taskEXIT_CRITICAL();
}

void LedEffectTask(void *arg) {
    LedMode last_mode = OFF;

    while (1) {
        // RTOS-safe read of current_mode preventing race
        taskENTER_CRITICAL();
        LedMode mode = current_mode;
        taskEXIT_CRITICAL();

        // Changing mode -> reset effect parameters
        if (mode != last_mode) {
            breathe_step = 0;
            flash_state = 0;
            stop_LED_effects();
            last_mode = mode;
        }

        switch (mode) {
            case BLUE:
                stop_LED_effects();
                turn_on_blue_LED();
                break;

            case GREEN:
                stop_LED_effects();
                turn_on_green_LED();
                break;

            case RED:
                stop_LED_effects();
                turn_on_red_LED();
                break;

            case RED_FLASH:
                stop_LED_effects();
                flash_state ^= 1;
                if (flash_state) turn_on_red_LED();
                else turn_off_red_LED();
                break;

            case RED_BREATHE:
                // Non-blocking breathing effect: advances brightness one step PER LOOP
                // using precomputed curve -> no delays or waits here, so the task
                // keeps running and cooperatively yields via vTaskDelay below.
                set_red_LED_brightness(breathe_curve[breathe_step]);
                breathe_step = (breathe_step + 1) % breathe_steps;
                break;

            case OFF:
                stop_LED_effects();
                break;

            default:
                stop_LED_effects();
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(60));
    }
}

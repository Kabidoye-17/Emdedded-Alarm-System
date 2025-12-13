#include "FreeRTOS.h"
#include "task.h"
#include "gpio.h"
#include "led_driver.h"
#include "alert_outputs.h"

static volatile LedMode current_mode = OFF;

static uint8_t breathe_step = 0;
static uint8_t flash_state = 0;

static const uint8_t breathe_curve[] = {
    0, 5, 10, 20, 40, 60, 80, 100, 80, 60, 40, 20, 10, 5
};
static const int breathe_steps = sizeof(breathe_curve) / sizeof(breathe_curve[0]);

void alert_outputs_set_mode(LedMode mode) {
    current_mode = mode;
}

static void LedEffectTask(void *arg) {
    LedMode last_mode = OFF;

    while (1) {
        if (current_mode != last_mode) {
            breathe_step = 0;
            flash_state = 0;
            stop_LED_effects();
            last_mode = current_mode;
        }

        switch (current_mode) {

        case BLUE:
            turn_on_blue_LED();
            break;

        case GREEN:
            turn_on_green_LED();
            break;

        case RED:
            turn_on_red_LED();
            break;

        case RED_FLASH:
            flash_state ^= 1;
            if (flash_state) turn_on_red_LED();
            else turn_off_red_LED();
            break;

        case RED_BREATHE:
            set_red_LED_brightness(breathe_curve[breathe_step]);
            breathe_step = (breathe_step + 1) % breathe_steps;
            break;

        case OFF:
        default:
            stop_LED_effects();
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(40));
    }
}

void alert_outputs_init() {
    xTaskCreate(
        LedEffectTask,
        "LEDEffects",
        512,
        NULL,
        1,
        NULL
    );
}
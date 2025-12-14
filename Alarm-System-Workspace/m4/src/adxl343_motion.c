#include "adxl343_motion.h"
#include "adxl343.h"
#include "gpio.h"
#include "mxc_device.h"
#include "board.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "typing.h"

/*
 * This module handles motion detection using the ADXL343 accelerometer.
 * Hardware interrupts from the sensor are converted into RTOS events
 * that higher-level system logic can react to.
 */

/***** Activity rate limiting *****/
/*
 * Activity interrupts can trigger continuously while movement persists.
 * This cooldown limits how often activity events are forwarded.
 */
#define ACTIVITY_COOLDOWN_MS 2000
static TickType_t last_activity_tick = 0;


/***** ADXL343 registers (motion related) *****/
/*
 * Register addresses taken directly from the ADXL343 datasheet.
 * These are used to configure tap, activity, inactivity and free-fall.
 */
#define ADXL343_THRESH_TAP     0x1D
#define ADXL343_DUR            0x21
#define ADXL343_LATENT         0x22
#define ADXL343_WINDOW         0x23
#define ADXL343_TAP_AXES       0x2A

#define ADXL343_THRESH_ACT     0x24
#define ADXL343_THRESH_INACT   0x25
#define ADXL343_TIME_INACT     0x26
#define ADXL343_ACT_INACT_CTL  0x27

#define ADXL343_THRESH_FF      0x28
#define ADXL343_TIME_FF        0x29

#define ADXL343_INT_ENABLE     0x2E
#define ADXL343_INT_MAP        0x2F
#define ADXL343_INT_SOURCE     0x30


/***** Interrupt source bits *****/
/*
 * Bit masks used to interpret the ADXL343 INT_SOURCE register.
 * Multiple bits may be set simultaneously.
 */
#define ADXL343_INT_DOUBLE_TAP (1 << 5)
#define ADXL343_INT_ACTIVITY   (1 << 4)
#define ADXL343_INT_INACTIVITY (1 << 3)
#define ADXL343_INT_FREE_FALL  (1 << 2)


/***** GPIO mapping *****/
/*
 * GPIO pin connected to the ADXL343 interrupt output.
 * The sensor asserts this pin when an enabled interrupt occurs.
 */
#define ADXL343_INT_PORT MXC_GPIO1
#define ADXL343_INT_PIN  8


/* ---------- RTOS objects ---------- */
/*
 * motionSem   -> binary semaphore used to wake the motion task from ISR
 * motionQueue -> queue used to send motion warning events to the system
 */
static SemaphoreHandle_t motionSem;
static QueueHandle_t motionQueue;


/* ---------- Module state ---------- */
/*
 * motion_flags accumulates interrupt source bits read from the sensor.
 * Marked volatile because it is written in ISR context and read in task context.
 */
static volatile uint8_t motion_flags = 0;


/***** GPIO ISR callback *****/
/*
 * This callback runs in interrupt context.
 * It should be fast and perform minimal work:
 *  - read interrupt source from sensor
 *  - store flags
 *  - wake motion task via semaphore
 */
static void gpio_irq_handler(void *cbdata)
{
    BaseType_t woken = pdFALSE;
    uint8_t src;

    (void)cbdata;

    // Reading INT_SOURCE clears the interrupt inside the ADXL343
    adxl343_read_regs(ADXL343_INT_SOURCE, &src, 1);

    // Accumulate interrupt flags (may receive multiple events)
    motion_flags |= src;

    // Wake motion detection task
    xSemaphoreGiveFromISR(motionSem, &woken);

    // Request context switch if needed
    portYIELD_FROM_ISR(woken);
}


/***** GPIO1 IRQ dispatcher *****/
/*
 * NVIC interrupt handler for GPIO1.
 * Delegates processing to the MAX GPIO driver,
 * which then calls the registered callback.
 */
void GPIO1_IRQHandler(void)
{
    MXC_GPIO_Handler(1);
}


/***** GPIO setup *****/
/*
 * Configures the GPIO pin connected to the ADXL343 interrupt line.
 * Interrupt is triggered on a rising edge.
 */
static void setup_gpio_interrupt(void)
{
    mxc_gpio_cfg_t cfg = {
        .port  = ADXL343_INT_PORT,
        .mask  = (1 << ADXL343_INT_PIN),
        .pad   = MXC_GPIO_PAD_NONE,
        .func  = MXC_GPIO_FUNC_IN,
        .vssel = MXC_GPIO_VSSEL_VDDIOH
    };

    // Configure GPIO pin as input
    MXC_GPIO_Config(&cfg);

    // Enable rising-edge interrupt
    MXC_GPIO_IntConfig(&cfg, MXC_GPIO_INT_RISING);

    // Register ISR callback
    MXC_GPIO_RegisterCallback(&cfg, gpio_irq_handler, NULL);

    // Enable GPIO interrupt
    MXC_GPIO_EnableInt(cfg.port, cfg.mask);

    // Enable GPIO interrupt at NVIC level
    NVIC_EnableIRQ(GPIO1_IRQn);
}


/***** Motion detection task *****/
/*
 * This task waits for motion interrupts signaled by the ISR.
 * It prioritizes events and sends the highest-priority warning
 * to the motion queue.
 */
static void MotionDetectionTask(void *arg)
{
    (void)arg;

    for (;;)
    {
        // Wait indefinitely for a motion interrupt
        xSemaphoreTake(motionSem, portMAX_DELAY);

        // Copy and clear accumulated interrupt flags
        uint8_t flags = motion_flags;
        motion_flags = 0;

        warn_type evt;
        uint8_t send = 1;

        /* Highest priority first */
        if (flags & ADXL343_INT_FREE_FALL)
        {
            // Free fall is treated as highest severity
            evt = HIGH_WARN;
        }
        else if (flags & ADXL343_INT_DOUBLE_TAP)
        {
            // Double tap is medium severity
            evt = MED_WARN;
        }
        else if (flags & ADXL343_INT_ACTIVITY)
        {
            // Rate-limit activity events to avoid queue flooding
            TickType_t now = xTaskGetTickCount();
            if ((now - last_activity_tick) >
                pdMS_TO_TICKS(ACTIVITY_COOLDOWN_MS))
            {
                evt = LOW_WARN;
                last_activity_tick = now;
            }
            else
            {
                send = 0;
            }
        }
        else
        {
            // No relevant event detected
            send = 0;
        }

        // Send event to queue if valid
        if (send)
        {
            motion_event motion = {evt};
            xQueueSend(motionQueue, &motion, 0);
        }
    }
}


/***** Public API *****/
/*
 * Initializes motion detection:
 *  - stores caller-provided queue
 *  - configures ADXL343 thresholds and interrupts
 *  - sets up GPIO interrupt
 *  - starts motion detection task
 */
int adxl343_motion_start(QueueHandle_t q,
                         UBaseType_t priority,
                         uint16_t stackDepth)
{
    // Store queue for motion events
    motionQueue = q;

    // Create binary semaphore for ISR-to-task signaling
    motionSem = xSemaphoreCreateBinary();
    if (!motionSem)
        return -1;

    /* ---------- Sensor configuration ---------- */

    // Tap detection configuration
    adxl343_write_reg(ADXL343_THRESH_TAP,   30);
    adxl343_write_reg(ADXL343_DUR,          20);
    adxl343_write_reg(ADXL343_LATENT,       40);
    adxl343_write_reg(ADXL343_WINDOW,      100);
    adxl343_write_reg(ADXL343_TAP_AXES,   0x07); // Enable X, Y, Z axes

    // Activity / inactivity configuration
    adxl343_write_reg(ADXL343_THRESH_ACT,   60);
    adxl343_write_reg(ADXL343_THRESH_INACT, 20);
    adxl343_write_reg(ADXL343_TIME_INACT,   50);
    adxl343_write_reg(ADXL343_ACT_INACT_CTL, 0x70);

    // Free-fall detection configuration
    adxl343_write_reg(ADXL343_THRESH_FF, 9);
    adxl343_write_reg(ADXL343_TIME_FF,  20);

    // Route all interrupts to INT1 pin
    adxl343_write_reg(ADXL343_INT_MAP, 0x00);

    // Enable desired interrupt sources
    adxl343_write_reg(
        ADXL343_INT_ENABLE,
        ADXL343_INT_DOUBLE_TAP |
        ADXL343_INT_ACTIVITY   |
        ADXL343_INT_INACTIVITY |
        ADXL343_INT_FREE_FALL
    );

    // Clear any latched interrupts
    uint8_t dummy;
    adxl343_read_regs(ADXL343_INT_SOURCE, &dummy, 1);

    // Configure GPIO interrupt for ADXL343 INT pin
    setup_gpio_interrupt();
    MXC_GPIO_ClearFlags(ADXL343_INT_PORT,
                        (1 << ADXL343_INT_PIN));

    // Create motion detection task
    return xTaskCreate(
        MotionDetectionTask,
        "MotionDetect",
        stackDepth,
        NULL,
        priority,
        NULL
    ) == pdPASS ? 0 : -2;
}

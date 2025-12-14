
#include "adxl343_motion.h"
#include "adxl343.h"
#include "gpio.h"
#include "mxc_device.h"
#include "board.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/***** ADXL343 registers *****/
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

/***** Interrupt bits *****/
#define ADXL343_INT_DOUBLE_TAP (1 << 5)
#define ADXL343_INT_ACTIVITY   (1 << 4)
#define ADXL343_INT_INACTIVITY (1 << 3)
#define ADXL343_INT_FREE_FALL  (1 << 2)

/***** GPIO mapping *****/
#define ADXL343_INT_PORT MXC_GPIO1
#define ADXL343_INT_PIN  8

/* ---------- RTOS objects ---------- */
static SemaphoreHandle_t motionSem;
static QueueHandle_t motionQueue;

/* ---------- Module state ---------- */
static volatile uint8_t motion_flags = 0;
static volatile uint8_t system_armed = 1;

/***** GPIO ISR callback *****/
static void gpio_irq_handler(void *cbdata)
{
    BaseType_t woken = pdFALSE;
    uint8_t src;

    adxl343_read_regs(ADXL343_INT_SOURCE, &src, 1);
    motion_flags |= src;

    xSemaphoreGiveFromISR(motionSem, &woken);
    portYIELD_FROM_ISR(woken);
}


/***** GPIO1 IRQ dispatcher *****/
void GPIO1_IRQHandler(void)
{
    MXC_GPIO_Handler(1);
}

/***** GPIO setup *****/
static void setup_gpio_interrupt(void)
{
    mxc_gpio_cfg_t cfg = {
        .port  = ADXL343_INT_PORT,
        .mask  = (1 << ADXL343_INT_PIN),
        .pad   = MXC_GPIO_PAD_NONE,
        .func  = MXC_GPIO_FUNC_IN,
        .vssel = MXC_GPIO_VSSEL_VDDIOH
    };

    MXC_GPIO_Config(&cfg);
    MXC_GPIO_IntConfig(&cfg, MXC_GPIO_INT_RISING);
    MXC_GPIO_RegisterCallback(&cfg, gpio_irq_handler, NULL);
    MXC_GPIO_EnableInt(cfg.port, cfg.mask);
    NVIC_EnableIRQ(GPIO1_IRQn);
}

/***** Motion task *****/
static void MotionDetectionTask(void *arg)
{

    (void)arg;

    for (;;)
    {
        xSemaphoreTake(motionSem, portMAX_DELAY);

        if (!system_armed)
        {
            motion_flags = 0;
            continue;
        }

        uint8_t flags = motion_flags;
        motion_flags = 0;

        MotionEvent evt;
        uint8_t send = 1;

        /* Highest priority first */
        if (flags & ADXL343_INT_FREE_FALL)
        {
            evt = HIGH_WARN;
        }
        else if (flags & ADXL343_INT_ACTIVITY)
        {
            evt = MED_WARN;
        }
        else if (flags & ADXL343_INT_DOUBLE_TAP)
        {
            evt = LOW_WARN;
        }
        // else if (flags & ADXL343_INT_INACTIVITY)
        // {
        //     evt = SAFE;
        // }
        else
        {
            send = 0;
        }

        if (send)
        { 
            xQueueSend(motionQueue, &evt, 0);
        }

    }
}

/***** Public API *****/
int adxl343_motion_start(QueueHandle_t q,
                         UBaseType_t priority,
                         uint16_t stackDepth)
{
    motionQueue = q;
    motionSem = xSemaphoreCreateBinary();
    if (!motionSem)
        return -1;

    /* Sensor config */
    adxl343_write_reg(ADXL343_THRESH_TAP, 45);   // ~2.8 g
    adxl343_write_reg(ADXL343_DUR,        15);   // ~9 ms
    adxl343_write_reg(ADXL343_LATENT,     30);   // ~30 ms
    adxl343_write_reg(ADXL343_WINDOW,     60);   // ~60 ms
    adxl343_write_reg(ADXL343_TAP_AXES,   0x07); // XYZ


    adxl343_write_reg(ADXL343_THRESH_ACT, 40); 
    adxl343_write_reg(ADXL343_THRESH_INACT, 15);
    adxl343_write_reg(ADXL343_TIME_INACT, 50);
    adxl343_write_reg(ADXL343_ACT_INACT_CTL, 0x70); 

    adxl343_write_reg(ADXL343_THRESH_FF, 9);
    adxl343_write_reg(ADXL343_TIME_FF, 20);

    adxl343_write_reg(ADXL343_INT_MAP, 0x00);
    adxl343_write_reg(ADXL343_INT_ENABLE,
        ADXL343_INT_DOUBLE_TAP |
        ADXL343_INT_ACTIVITY   |
        ADXL343_INT_INACTIVITY |
        ADXL343_INT_FREE_FALL);

    uint8_t dummy;
    adxl343_read_regs(ADXL343_INT_SOURCE, &dummy, 1);

    setup_gpio_interrupt();
    MXC_GPIO_ClearFlags(ADXL343_INT_PORT, (1 << ADXL343_INT_PIN));

    return xTaskCreate(
        MotionDetectionTask,
        "MotionDetect",
        stackDepth,
        NULL,
        priority,
        NULL
    ) == pdPASS ? 0 : -2;
}


#include "mxc_device.h"
#include "uart.h"
#include "nvic_table.h"
#include <stddef.h>

#include "uart_coms.h"

#define BAUD_RATE 115200
#define JSON_BUFFER_SIZE 96

typedef struct {
    uart_rxMessage_cbt uart_rxMessage_cb;
    char json_buffer[JSON_BUFFER_SIZE];
    volatile int json_idx;
} uart_vars_t;
static uart_vars_t uart_vars;


/**
 * @brief UART0 interrupt handler
 *
 * Handles incoming UART data byte-by-byte, buffering JSON messages between
 * '{' and '}' delimiters. Calls registered callback when complete message received.
 *
 * Message parsing:
 * - '{' resets buffer index to start new message
 * - '}' null-terminates buffer and invokes callback
 * - Other bytes are stored in buffer (up to JSON_BUFFER_SIZE - 1 chars)
 */
void UART0_Handler(void)
{
    if (MXC_UART_GetFlags(MXC_UART0) & MXC_F_UART_INT_FL_RX_THD) {
        uint8_t byte;
        MXC_UART_ReadRXFIFO(MXC_UART0, &byte, 1);
        MXC_UART_ClearFlags(MXC_UART0, MXC_F_UART_INT_FL_RX_THD);

        if (byte == '{') {
            uart_vars.json_idx = 0;
        }
        else if (byte == '}') {
            uart_vars.json_buffer[uart_vars.json_idx] = '\0';
            if (uart_vars.uart_rxMessage_cb != NULL) {
                uart_vars.uart_rxMessage_cb(uart_vars.json_buffer);
            }
        }
        else if (uart_vars.json_idx < JSON_BUFFER_SIZE - 1) {
            uart_vars.json_buffer[uart_vars.json_idx++] = byte;
        }
    }
}

/**
 * @brief Initialize UART0 for receiving JSON messages
 *
 * Configures UART0 at 115200 baud with RX interrupt enabled. Registers a callback
 * function to be invoked when complete JSON messages are received.
 *
 * @param uart_rxMessage_cb Callback function to handle complete JSON messages
 *
 * Setup steps:
 * 1. Register callback and reset buffer index
 * 2. Configure NVIC for UART0 interrupts
 * 3. Initialize UART0 hardware at BAUD_RATE (115200)
 * 4. Enable RX threshold interrupt
 *
 * @note Halts execution on UART initialization error
 */
void uart_init(uart_rxMessage_cbt uart_rxMessage_cb)
{
    uart_vars.uart_rxMessage_cb = uart_rxMessage_cb;
    uart_vars.json_idx = 0;

    NVIC_DisableIRQ(UART0_IRQn);
    NVIC_ClearPendingIRQ(UART0_IRQn);
    MXC_NVIC_SetVector(UART0_IRQn, UART0_Handler);
    NVIC_EnableIRQ(UART0_IRQn);

    int error;
    if ((error = MXC_UART_Init(MXC_UART0, BAUD_RATE, MXC_UART_IBRO_CLK)) != E_NO_ERROR) {
        while(1);  // Halt on error
    }
    MXC_UART_EnableInt(MXC_UART0, MXC_F_UART_INT_EN_RX_THD);
}

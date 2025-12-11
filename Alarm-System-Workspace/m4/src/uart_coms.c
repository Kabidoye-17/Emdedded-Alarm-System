#include "mxc_device.h"
#include "uart.h"
#include "nvic_table.h"
#include <stddef.h>
#include <string.h>

#include "uart_coms.h"
#include "crc.h"

#define BAUD_RATE 115200
#define PROTOCOL_STX 0x02
#define PROTOCOL_ETX 0x03
#define MAX_DATA_LENGTH 16

typedef enum {
    STATE_WAIT_STX,      // Waiting for STX (0x02)
    STATE_READ_LENGTH,   // Reading length byte
    STATE_READ_DATA,     // Reading data bytes
    STATE_READ_CRC_LOW,  // Reading CRC low byte
    STATE_READ_CRC_HIGH, // Reading CRC high byte
    STATE_WAIT_ETX       // Waiting for ETX (0x03)
} uart_rx_state_t;

typedef struct {
    uart_rxMessage_cbt uart_rxMessage_cb;
    uart_rx_state_t state;
    uint8_t data_buffer[MAX_DATA_LENGTH];
    uint8_t data_length;
    uint8_t data_index;
    uint16_t calculated_crc;
    uint16_t received_crc;
} uart_vars_t;

static uart_vars_t uart_vars;

/**
 * @brief Parse command string to command_type enum
 *
 * @param data Pointer to command data buffer
 * @param length Length of command string
 * @return command_type enum value or UNKNOWN_COMMAND if not recognized
 */
static command_type parse_command(const uint8_t* data, uint8_t length)
{
    char cmd_str[MAX_DATA_LENGTH + 1];
    memcpy(cmd_str, data, length);
    cmd_str[length] = '\0';

    if (strcmp(cmd_str, "ARM") == 0) {
        return ARM;
    } else if (strcmp(cmd_str, "DISARM") == 0) {
        return DISARM;
    } else if (strcmp(cmd_str, "RESOLVE") == 0) {
        return RESOLVE;
    }

    return UNKNOWN_COMMAND;
}

/**
 * @brief UART0 interrupt handler with state machine for STX/ETX frame parsing
 *
 * Handles incoming UART data byte-by-byte using state machine to parse binary frames.
 * Frame format: [STX][length][data...][crc_low][crc_high][ETX]
 *
 * State transitions:
 * - WAIT_STX: Wait for STX (0x02), initialize CRC
 * - READ_LENGTH: Read and validate length byte (1-16)
 * - READ_DATA: Accumulate data bytes, update CRC
 * - READ_CRC_LOW: Read CRC low byte
 * - READ_CRC_HIGH: Read CRC high byte
 * - WAIT_ETX: Validate ETX and CRC, invoke callback if valid
 *
 * Invalid frames are silently discarded.
 */
void UART0_Handler(void)
{
    if (MXC_UART_GetFlags(MXC_UART0) & MXC_F_UART_INT_FL_RX_THD) {
        uint8_t byte;
        MXC_UART_ReadRXFIFO(MXC_UART0, &byte, 1);
        MXC_UART_ClearFlags(MXC_UART0, MXC_F_UART_INT_FL_RX_THD);

        switch (uart_vars.state) {
            case STATE_WAIT_STX:
                if (byte == PROTOCOL_STX) {
                    uart_vars.state = STATE_READ_LENGTH;
                    uart_vars.calculated_crc = CRCINIT;
                    memset(uart_vars.data_buffer, 0, MAX_DATA_LENGTH);
                }
                break;

            case STATE_READ_LENGTH:
                uart_vars.data_length = byte;
                uart_vars.data_index = 0;
                uart_vars.calculated_crc = crc_iterate(uart_vars.calculated_crc, byte);

                if (uart_vars.data_length > 0 && uart_vars.data_length <= MAX_DATA_LENGTH) {
                    uart_vars.state = STATE_READ_DATA;
                } else {
                    // Invalid length, reset state machine
                    uart_vars.state = STATE_WAIT_STX;
                }
                break;

            case STATE_READ_DATA:
                uart_vars.data_buffer[uart_vars.data_index++] = byte;
                uart_vars.calculated_crc = crc_iterate(uart_vars.calculated_crc, byte);

                if (uart_vars.data_index >= uart_vars.data_length) {
                    uart_vars.state = STATE_READ_CRC_LOW;
                }
                break;

            case STATE_READ_CRC_LOW:
                uart_vars.received_crc = byte;  // Low byte
                uart_vars.state = STATE_READ_CRC_HIGH;
                break;

            case STATE_READ_CRC_HIGH:
                uart_vars.received_crc |= (byte << 8);  // High byte
                uart_vars.state = STATE_WAIT_ETX;
                break;

            case STATE_WAIT_ETX:
                if (byte == PROTOCOL_ETX) {
                    // Validate CRC
                    if (uart_vars.calculated_crc == uart_vars.received_crc) {
                        // Parse command and invoke callback
                        command_type cmd = parse_command(uart_vars.data_buffer, uart_vars.data_length);
                        if (cmd != UNKNOWN_COMMAND && uart_vars.uart_rxMessage_cb != NULL) {
                            uart_vars.uart_rxMessage_cb(cmd);
                        }
                    }
                    // Silently discard on CRC error (as per spec)
                }
                // Always reset to wait for next frame
                uart_vars.state = STATE_WAIT_STX;
                break;
        }
    }
}

/**
 * @brief Initialize UART0 for receiving binary framed messages
 *
 * Configures UART0 at 115200 baud with RX interrupt enabled. Registers a callback
 * function to be invoked when valid command frames are received.
 *
 * @param uart_rxMessage_cb Callback function to handle received commands
 *
 * Setup steps:
 * 1. Register callback and initialize state machine
 * 2. Configure NVIC for UART0 interrupts
 * 3. Initialize UART0 hardware at BAUD_RATE (115200)
 * 4. Enable RX threshold interrupt
 *
 * @note Halts execution on UART initialization error
 */
void uart_init(uart_rxMessage_cbt uart_rxMessage_cb)
{
    uart_vars.uart_rxMessage_cb = uart_rxMessage_cb;
    uart_vars.state = STATE_WAIT_STX;

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

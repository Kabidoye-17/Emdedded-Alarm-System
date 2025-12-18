#include "mxc_device.h"
#include "uart.h"
#include "nvic_table.h"
#include <stddef.h>
#include <string.h>

#include "uart_coms.h"
#include "crc.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "cloud_tasks.h"

#define BAUD_RATE 115200
#define PROTOCOL_STX 0x02
#define PROTOCOL_ETX 0x03
#define ACK_BYTE 0xAA
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
        return RESOLVE_ALARM;
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
                // Idle state - waiting for frame start marker or standalone ACK
                if (byte == PROTOCOL_STX) {
                    // Start of frame detected - prepare to receive new message
                    uart_vars.state = STATE_READ_LENGTH;

                    // Initialize CRC calculation (all frames include length and data in CRC)
                    uart_vars.calculated_crc = CRCINIT;

                    // Clear data buffer to ensure clean state for new frame
                    memset(uart_vars.data_buffer, 0, MAX_DATA_LENGTH);

                } else if (byte == ACK_BYTE) {
                    // Standalone ACK byte (0xAA) received from gateway
                    // This acknowledges a frame we previously sent
                    // Signal the cloud_send_task via semaphore to unblock waiting
                    on_ack_received();
                }
                // Any other byte: noise or out-of-sync data - ignore and stay in WAIT_STX
                break;

            case STATE_READ_LENGTH:
                // Read the length byte which tells us how many data bytes to expect
                uart_vars.data_length = byte;
                uart_vars.data_index = 0;  // Reset buffer index for incoming data

                // Length byte is included in CRC calculation
                uart_vars.calculated_crc = crc_iterate(uart_vars.calculated_crc, byte);

                // Validate length is within acceptable range (1-16 bytes)
                if (uart_vars.data_length > 0 && uart_vars.data_length <= MAX_DATA_LENGTH) {
                    // Valid length - proceed to read data bytes
                    uart_vars.state = STATE_READ_DATA;
                } else {
                    // Invalid length (0 or >16) - malformed frame
                    // Abort and return to idle state to resynchronize
                    uart_vars.state = STATE_WAIT_STX;
                }
                break;

            case STATE_READ_DATA:
                // Accumulate data bytes into buffer while updating CRC
                // Store the byte in the buffer at the current index (then increment index)
                uart_vars.data_buffer[uart_vars.data_index++] = byte;

                // Update running CRC calculation with this data byte
                uart_vars.calculated_crc = crc_iterate(uart_vars.calculated_crc, byte);

                // Check if we've received all expected data bytes
                if (uart_vars.data_index >= uart_vars.data_length) {
                    // All data received - next bytes will be CRC (low byte first)
                    uart_vars.state = STATE_READ_CRC_LOW;
                }
                break;

            case STATE_READ_CRC_LOW:
                // Read the low byte (LSB) of the 16-bit CRC sent by transmitter
                // CRC is transmitted little-endian: low byte first, high byte second
                uart_vars.received_crc = byte;  // Store low 8 bits
                uart_vars.state = STATE_READ_CRC_HIGH;
                break;

            case STATE_READ_CRC_HIGH:
                // Read the high byte (MSB) of the 16-bit CRC
                // Combine with low byte to form complete 16-bit CRC value
                uart_vars.received_crc |= (byte << 8);  // Shift high byte left, OR with low byte
                uart_vars.state = STATE_WAIT_ETX;
                break;

            case STATE_WAIT_ETX:
                // Expecting ETX frame terminator - validate and process if present
                if (byte == PROTOCOL_ETX) {
                    // Valid frame terminator received - now validate CRC checksum
                    if (uart_vars.calculated_crc == uart_vars.received_crc) {
                        // CRC matches - frame is valid, parse the command
                        command_type cmd = parse_command(uart_vars.data_buffer, uart_vars.data_length);

                        // Invoke callback if command is recognized and callback is registered
                        if (cmd != UNKNOWN_COMMAND && uart_vars.uart_rxMessage_cb != NULL) {
                            uart_vars.uart_rxMessage_cb(cmd);
                        }
                    }
                    // If CRC mismatch: silently discard frame (as per spec)
                    // This prevents acting on corrupted data
                }
                // If byte != ETX: frame is malformed, discard

                // Always reset state machine to wait for next frame
                // Even on error, we return to idle state to resynchronize
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
    
    // Flush RX/TX FIFOs to clear any bootloader noise or stale data
    MXC_UART_ClearRXFIFO(MXC_UART0);
    MXC_UART_ClearTXFIFO(MXC_UART0);
    
    // Set RX threshold to 1 byte to trigger interrupt on each received byte
    MXC_UART_SetRXThreshold(MXC_UART0, 1);

    MXC_UART_EnableInt(MXC_UART0, MXC_F_UART_INT_EN_RX_THD);
}

/**
 * @brief Transmit single byte over UART0 with timeout detection
 *
 * @param byte Byte to transmit
 * @param timeout_ms Timeout in milliseconds
 * @return 0 on success, -1 on timeout (gateway offline)
 */
static int uart_txByte_with_timeout(uint8_t byte, uint32_t timeout_ms) {
    TickType_t start_tick = xTaskGetTickCount();
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);

    while (MXC_UART_GetTXFIFOAvailable(MXC_UART0) == 0) {
        // Check if timeout exceeded
        if ((xTaskGetTickCount() - start_tick) >= timeout_ticks) {
            return -1;  // Timeout - assume gateway offline
        }

        // Yield CPU to prevent busy-wait
        vTaskDelay(1);
    }

    // FIFO space available - transmit byte
    MXC_UART_WriteTXFIFO(MXC_UART0, &byte, 1);
    return 0;
}


/**
 * @brief Build and transmit framed message with timeout detection
 *
 * Frame format: [STX][length][data...][crc_low][crc_high][ETX]
 *
 * @param data Pointer to data buffer
 * @param length Number of data bytes (1-MAX_DATA_LENGTH)
 * @param timeout_ms Timeout for each byte transmission
 * @return 0 on success, -1 on invalid length or timeout
 */
int uart_send_frame_with_timeout(const uint8_t* data, uint8_t length, uint32_t timeout_ms) {
    if (length == 0 || length > MAX_DATA_LENGTH) {
        return -1;  // Invalid length
    }

    // Calculate CRC over [length][data]
    uint16_t crc = CRCINIT;
    crc = crc_iterate(crc, length);
    for (uint8_t i = 0; i < length; i++) {
        crc = crc_iterate(crc, data[i]);
    }

    uint8_t crc_low = crc & 0xFF;
    uint8_t crc_high = (crc >> 8) & 0xFF;

    // Transmit frame with timeout checks
    if (uart_txByte_with_timeout(PROTOCOL_STX, timeout_ms) != 0) return -1;
    if (uart_txByte_with_timeout(length, timeout_ms) != 0) return -1;

    for (uint8_t i = 0; i < length; i++) {
        if (uart_txByte_with_timeout(data[i], timeout_ms) != 0) return -1;
    }

    if (uart_txByte_with_timeout(crc_low, timeout_ms) != 0) return -1;
    if (uart_txByte_with_timeout(crc_high, timeout_ms) != 0) return -1;
    if (uart_txByte_with_timeout(PROTOCOL_ETX, timeout_ms) != 0) return -1;

    return 0;  // Success
}

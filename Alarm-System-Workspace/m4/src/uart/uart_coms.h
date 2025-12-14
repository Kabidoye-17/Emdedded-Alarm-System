#ifndef __UART_H
#define __UART_H

#include <stdint.h>
#include "../utils/typing.h"

typedef void (*uart_rxMessage_cbt)(command_type cmd);
void uart_init(uart_rxMessage_cbt uart_rxMessage_cb);
int uart_send_frame_with_timeout(const uint8_t* data, uint8_t length, uint32_t timeout_ms);

#endif

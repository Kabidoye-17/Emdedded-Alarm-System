#ifndef __UART_H
#define __UART_H

#include <stdint.h>

typedef void (*uart_rxMessage_cbt)(const char* json_str);
void uart_init(uart_rxMessage_cbt uart_rxMessage_cb);

#endif

#ifndef __UART_H
#define __UART_H

#include <stdint.h>
#include "command.h"

typedef void (*uart_rxMessage_cbt)(command_type cmd);
void uart_init(uart_rxMessage_cbt uart_rxMessage_cb);

#endif

/**
 * Configuration for MAX32655-CTBGA.
 *
 * This file was generated using Analog Devices CodeFusion Studio.
 * https://github.com/analogdevicesinc/codefusion-studio
 *
 * Generated at: 2025-12-08T20:17:01.723Z *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2025 Analog Devices, Inc.
 */


#include <mxc_device.h>
#include <mxc_sys.h>
#include <stddef.h>
#include <uart.h>
#include "soc_init.h"

int PinInit(void) {
  int result;

  /* Initialize all the used GPIO Ports.
  */
  result = MXC_GPIO_Init(MXC_GPIO_PORT_0);
  if (result != E_NO_ERROR) {
    return result;
  }

  MXC_GPIO_SetConfigLock(MXC_GPIO_CONFIG_UNLOCKED);

  /* P0.2 (F8): assigned to GPIO0_P0.2.
  */
  const mxc_gpio_cfg_t cfg_p0_2 = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_2,
    .func = MXC_GPIO_FUNC_IN,
    .pad = MXC_GPIO_PAD_PULL_UP,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0 
  };
  result = MXC_GPIO_Config(&cfg_p0_2);
  if (result != E_NO_ERROR) {
    return result;
  }

  /* P0.3 (F9): assigned to GPIO0_P0.3.
  */
  const mxc_gpio_cfg_t cfg_p0_3 = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_3,
    .func = MXC_GPIO_FUNC_IN,
    .pad = MXC_GPIO_PAD_PULL_UP,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0 
  };
  result = MXC_GPIO_Config(&cfg_p0_3);
  if (result != E_NO_ERROR) {
    return result;
  }

  /* P0.18 (H5): assigned to GPIO0_P0.18.
  */
  const mxc_gpio_cfg_t cfg_p0_18 = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_18,
    .func = MXC_GPIO_FUNC_OUT,
    .pad = MXC_GPIO_PAD_NONE,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0 
  };
  result = MXC_GPIO_Config(&cfg_p0_18);
  if (result != E_NO_ERROR) {
    return result;
  }

  /* P0.19 (G4): assigned to GPIO0_P0.19.
  */
  const mxc_gpio_cfg_t cfg_p0_19 = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_19,
    .func = MXC_GPIO_FUNC_OUT,
    .pad = MXC_GPIO_PAD_NONE,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0 
  };
  result = MXC_GPIO_Config(&cfg_p0_19);
  if (result != E_NO_ERROR) {
    return result;
  }

  /* P0.26 (H1): assigned to GPIO0_P0.26.
  */
  const mxc_gpio_cfg_t cfg_p0_26 = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_26,
    .func = MXC_GPIO_FUNC_OUT,
    .pad = MXC_GPIO_PAD_NONE,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0 
  };
  result = MXC_GPIO_Config(&cfg_p0_26);
  if (result != E_NO_ERROR) {
    return result;
  }

  /* P0.0 (F7): assigned to UART0_RX.
  */
  const mxc_gpio_cfg_t cfg_p0_0 = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_0,
    .func = MXC_GPIO_FUNC_ALT1,
    .pad = MXC_GPIO_PAD_WEAK_PULL_UP,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0 
  };
  result = MXC_GPIO_Config(&cfg_p0_0);
  if (result != E_NO_ERROR) {
    return result;
  }

  /* P0.1 (E7): assigned to UART0_TX.
  */
  const mxc_gpio_cfg_t cfg_p0_1 = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_1,
    .func = MXC_GPIO_FUNC_ALT1,
    .pad = MXC_GPIO_PAD_NONE,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0 
  };
  result = MXC_GPIO_Config(&cfg_p0_1);
  if (result != E_NO_ERROR) {
    return result;
  }

  MXC_GPIO_SetConfigLock(MXC_GPIO_CONFIG_LOCKED);

  return E_NO_ERROR;
}

int PeripheralInit(void) {
  int result = E_NO_ERROR;

  /* SYS_OSC Mux: Clock Source is set to IPO (Internal Primary Osc.). */
  result = MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
  if (result != E_NO_ERROR) {
    return result;
  }

  { /* Configure UART0.
     */

    /* Initialize the peripheral. */
    result = MXC_UART_Init(MXC_UART0,
                           115200U,
                           MXC_UART_IBRO_CLK);
    if (result != E_NO_ERROR) {
      return result;
    }

    /* Set Data Size. */
    result = MXC_UART_SetDataSize(MXC_UART0, 8);
    if (result != E_NO_ERROR) {
      return result;
    }

    /* Set Stop Bits. */
    result = MXC_UART_SetStopBits(MXC_UART0, MXC_UART_STOP_1);
    if (result != E_NO_ERROR) {
      return result;
    }

    /* Set Flow Control. */
    result = MXC_UART_SetFlowCtrl(MXC_UART0, MXC_UART_FLOW_DIS, 1);
    if (result != E_NO_ERROR) {
      return result;
    }

    /* Set Parity. */
    result = MXC_UART_SetParity(MXC_UART0, MXC_UART_PARITY_DISABLE);
    if (result != E_NO_ERROR) {
      return result;
    }

  }

  /* Update the System Core Clock as core clock settings have changed. */
  SystemCoreClockUpdate();


  return result;
}

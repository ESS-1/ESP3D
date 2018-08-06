/*
  boards/wf3d_with_ssd1306.h - WF3D with SSD1306 display board definition.

  Copyright (c) 2018 Eugene Shelkovin. All rights reserved.
  This file is distributed under MIT license.
  MIT license may not be applied to the whole software or its files which
  are not explicitly marked as those distributed under MIT license.
*/

#pragma once

#define PIN_OUT_LED_R         (16)
#define PIN_OUT_LED_G         (12)
#define PIN_OUT_LED_B         (13)
#define PIN_OUT_UART_SWITCH   (4)
#define PIN_OUT_PRINTER_RESET (15)
#define PIN_IN_SETTINGS_RESET (0)

#define BRD_POWERON_DELAY     (3000)

#define DISPLAY_SSD1306
#define DISPLAY_I2C_ADDR      (0x3C)
#define DISPLAY_I2C_SDA       (5)
#define DISPLAY_I2C_SCL       (14)

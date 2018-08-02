/*
  board.h - hardware abstraction.

  Copyright (c) 2018 Eugene Shelkovin. All rights reserved.
  This file is distributed under MIT license.
  MIT license may not be applied to the whole software or its files which
  are not explicitly marked as those distributed under MIT license.
*/

#pragma once

#include <Arduino.h>
#include "display.h"
#include "devices.h"

// StatusController
class StatusController
{
private:
    void updateSummary();
    void updateLeds();

public:
    void init();
    void update();

    void print(const char *status, bool displayInLogOnly = false);
    void printOver(const char *status, bool displayInLogOnly = false);

    inline void print(const String &status, bool displayInLogOnly = false)
    {
        print(status.c_str(), displayInLogOnly);
    }

    inline void printOver(const String &status, bool displayInLogOnly = false)
    {
        printOver(status.c_str(), displayInLogOnly);
    }
};


// Board
class Board
{
public:
    static void resetSettings();

    static HardwareSerial& printerPort;
    static StatusController status;

    static SimpleGpioOutputDevice* const pPrinterPortSwitch;
    static GpioOutputDevice* const pPrinterReset;
    static HoldButton* const pResetButton;

    static GpioOutputDevice* const pLedR;
    static GpioOutputDevice* const pLedG;
    static GpioOutputDevice* const pLedB;
    static Display* const pDisplay;

    static void init();
    static void update();
    static bool isPinUsed(uint8_t pin);
};


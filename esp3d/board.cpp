/*
  board.cpp - hardware abstraction.

  Copyright (c) 2018 Eugene Shelkovin. All rights reserved.
  This file is distributed under MIT license.
  MIT license may not be applied to the whole software or its files which
  are not explicitly marked as those distributed under MIT license.
*/

#pragma once

#include "board.h"
#include "config.h"


// StatusController
const char M117_[] PROGMEM = "M117 ";

void StatusController::updateSummary()
{
    if (_updateTimer.milliSeconds() < 100)
    { // Do not update summary more often than 100 ms
        return;
    }
    _updateTimer.restart();

    switch (WiFi.getMode())
    {
        case WIFI_STA:
            {
                wl_status_t status = WiFi.status();
                switch (status)
                {
                    case WL_CONNECTED:
                        updateStatus(WiFiIcon_Sta, WiFi.localIP().toString(), status, true);
                        break;
                    case WL_NO_SSID_AVAIL:
                        updateStatus(WiFiIcon_Off, F("No SSID found"), status, true);
                        break;
                    case WL_CONNECT_FAILED:
                        updateStatus(WiFiIcon_Off, F("Connection failed"), status, true);
                        break;
                    case WL_CONNECTION_LOST:
                        updateStatus(WiFiIcon_Off, F("Connection lost"), status, true);
                        break;
                    default:
                        updateStatus(WiFiIcon_Sta, F("..."), status);
                        break;
                }
            }
            break;

        case WIFI_AP:
        case WIFI_AP_STA:
            {
                char sbuf[MAX_SSID_LENGTH+1];
                if (CONFIG::read_string(EP_AP_SSID, sbuf, MAX_SSID_LENGTH))
                    updateStatus(WiFiIcon_Ap, sbuf, WL_DISCONNECTED);
                else
                    updateStatus(WiFiIcon_Ap, F("Can't get SSID"), WL_DISCONNECTED);
            }
            break;

        default:
            updateStatus(WiFiIcon_Off, F("WiFi off"), WL_DISCONNECTED);
            break;
    }
}

void StatusController::updateStatus(WiFiIcon icon, const String & descr, wl_status_t newStaStatus, bool logStaStatusChange /*=false*/)
{
    if (Board::pDisplay != NULL)
    {
        Board::pDisplay->printSummary(icon, descr);
    }

    bool staStatusChanged = (newStaStatus != _lastStaStatus);
    _lastStaStatus = newStaStatus;

    if (logStaStatusChange && staStatusChanged)
    {
        print(descr);
    }
}

void StatusController::updateLeds()
{
    if (Board::pLedR != NULL) // Error LED
    {
        WiFiMode_t mode = WiFi.getMode();
        if (mode == WIFI_STA)
        {
            switch (WiFi.status())
            {
                case WL_NO_SSID_AVAIL:
                case WL_CONNECT_FAILED:
                case WL_CONNECTION_LOST:
                    Board::pLedR->blink(500);
                    break;
                default:
                    Board::pLedR->off();
                    break;
            }
        }
        else
        {
            Board::pLedR->off();
        }
    }
    if (Board::pLedB != NULL) // WiFi LED
    {
        WiFiMode_t mode = WiFi.getMode();
        if (mode == WIFI_STA && WiFi.isConnected())
            Board::pLedB->on();
        else if (mode == WIFI_AP || mode == WIFI_AP_STA)
            Board::pLedB->blink(1500);
        else
            Board::pLedB->off();
    }
}

void StatusController::init()
{
    if (Board::pLedG != NULL) // Heartbeat LED
        Board::pLedG->blink(1500);

    if (Board::pLedR != NULL) Board::pLedR->pulse(1000);
    if (Board::pLedB != NULL) Board::pLedB->pulse(1000);
}

void StatusController::update()
{
    updateSummary();
    updateLeds();
}

void StatusController::print(const char *status, bool displayInLogOnly /*=false*/)
{
    if (Board::pDisplay != NULL)
    {
        Board::pDisplay->newLine();
    }

    printOver(status, displayInLogOnly);
}

void StatusController::printOver(const char *status, bool displayInLogOnly /*=false*/)
{
    if (!displayInLogOnly)
    {
        Board::printerPort.print(FPSTR(M117_));
        Board::printerPort.println(status);
    }

    if (Board::pDisplay != NULL)
    {
        // print a new status line over the previous status line.
        Board::pDisplay->print(status);
    }
}


// Board - devices initialization
HardwareSerial& Board::printerPort = Serial;

StatusController Board::status = StatusController();

#ifdef PIN_OUT_UART_SWITCH
    SimpleGpioOutputDevice cPrinterPortSwitch(PIN_OUT_UART_SWITCH);
    SimpleGpioOutputDevice* const Board::pPrinterPortSwitch = &cPrinterPortSwitch;
#else
    SimpleGpioOutputDevice* const Board::pPrinterPortSwitch = NULL;
#endif

#ifdef PIN_OUT_PRINTER_RESET
    GpioOutputDevice cPrinterReset(PIN_OUT_PRINTER_RESET, HIGH);
    GpioOutputDevice* const Board::pPrinterReset = &cPrinterReset;
#else
    GpioOutputDevice* const Board::pPrinterReset = NULL;
#endif

#ifdef PIN_IN_SETTINGS_RESET
    HOLDBUTTON_DECLARE_ISR(PIN_IN_SETTINGS_RESET)
    HoldButton cResetButton(HOLDBUTTON_ISR(PIN_IN_SETTINGS_RESET), Board::resetSettings, 10000);
    HoldButton* const Board::pResetButton = &cResetButton;
#else
    HoldButton* const Board::pResetButton = NULL;
#endif

#ifdef PIN_OUT_LED_R
    GpioOutputDevice cLedR(PIN_OUT_LED_R);
    GpioOutputDevice* const Board::pLedR = &cLedR;
#else
    GpioOutputDevice* const Board::pLedR = NULL;
#endif

#ifdef PIN_OUT_LED_G
    GpioOutputDevice cLedG(PIN_OUT_LED_G);
    GpioOutputDevice* const Board::pLedG = &cLedG;
#else
    GpioOutputDevice* const Board::pLedG = NULL;
#endif

#ifdef PIN_OUT_LED_B
    GpioOutputDevice cLedB(PIN_OUT_LED_B);
    GpioOutputDevice* const Board::pLedB = &cLedB;
#else
    GpioOutputDevice* const Board::pLedB = NULL;
#endif

#ifdef DISPLAY_SSD1306
#   include "display_ssd1306.h"
    DisplaySSD1306 cDisplay(DISPLAY_I2C_ADDR, DISPLAY_I2C_SDA, DISPLAY_I2C_SCL);
    Display* const Board::pDisplay = &cDisplay;
#else
    Display* const Board::pDisplay = NULL;
#endif


// Board - methods
void Board::resetSettings()
{
    if (pLedR != NULL) pLedR->on();

    // Give a user 2.5 seconds to release the button
    status.print(F("Hold to clr. cfg."));
    delay(2500);
    if (pResetButton != NULL &&
        !pResetButton->isPressed())
    {
        status.print(F("Canceled"));
        return;
    }

    CONFIG::reset_config();
    CONFIG::esp_restart();
}

void Board::init()
{
    if (pDisplay != NULL) pDisplay->init();
    status.init();
}

void Board::update()
{
    if (pPrinterReset != NULL) pPrinterReset->update();
    if (pResetButton != NULL) pResetButton->update();
    status.update();
    if (pLedR != NULL) pLedR->update();
    if (pLedG != NULL) pLedG->update();
    if (pLedB != NULL) pLedB->update();
    if (pDisplay != NULL) pDisplay->update();
}

bool Board::isPinUsed(uint8_t pin)
{
    return (pin == 1 || pin == 3) || // UART pins
           (pPrinterPortSwitch != NULL && pPrinterPortSwitch->getPin() == pin) ||
           (pPrinterReset != NULL && pPrinterReset->getPin() == pin) ||
           (pResetButton != NULL && pResetButton->getPin() == pin) ||
           (pLedR != NULL && pLedR->getPin() == pin) ||
           (pLedG != NULL && pLedG->getPin() == pin) ||
           (pLedB != NULL && pLedB->getPin() == pin) ||
           (pDisplay != NULL && pDisplay->isPinUsed(pin));
}


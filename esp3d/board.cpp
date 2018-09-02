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
    // Get WiFi status
    WiFiIcon icon;
    wl_status_t newStaStatus;
    bool logStaStatusChange;
    String summary = getWiFiStatus(&icon, &newStaStatus, &logStaStatusChange);

    if (logStaStatusChange && (newStaStatus != _lastStaStatus))
    {
        print(summary);
    }

    // Get VMon alarm status
    VoltageMonitorStatus newVMonStatus = Board::pVoltageMonitor != NULL
        ? Board::pVoltageMonitor->getStatus()
        : VMonStatus_Ok;
    if (newVMonStatus == VMonStatus_Undervoltage || newVMonStatus == VMonStatus_Overvoltage)
    {
        summary = String(newVMonStatus == VMonStatus_Undervoltage
                     ? F("Undervoltage ")
                     : F("Overvoltage ")) +
                 Board::pVoltageMonitor->formatVoltage();
        if (newVMonStatus != _lastVMonStatus)
        {
            print(summary);
        }
    }

    // Display status
    if (Board::pDisplay != NULL)
    {
        Board::pDisplay->printSummary(icon, summary);
    }

    _lastStaStatus = newStaStatus;
    _lastVMonStatus = newVMonStatus;
}

String StatusController::getWiFiStatus(WiFiIcon *pIcon, wl_status_t *pStaStatus, bool *pLogStatusChange)
{
    *pIcon = WiFiIcon_Off;
    *pStaStatus = WL_DISCONNECTED;
    *pLogStatusChange = false;

    switch (WiFi.getMode())
    {
        case WIFI_STA:
            {
                *pStaStatus = WiFi.status();
                switch (*pStaStatus)
                {
                    case WL_CONNECTED:
                        *pIcon = WiFiIcon_Sta;
                        *pLogStatusChange = true;
                        return WiFi.localIP().toString();
                    case WL_NO_SSID_AVAIL:
                        *pLogStatusChange = true;
                        return F("No SSID found");
                    case WL_CONNECT_FAILED:
                        *pLogStatusChange = true;
                        return F("Connection failed");
                    case WL_CONNECTION_LOST:
                        *pLogStatusChange = true;
                        return F("Connection lost");
                    default:
                        *pIcon = WiFiIcon_Sta;
                        return F("...");
                }
            }

        case WIFI_AP:
        case WIFI_AP_STA:
            {
                *pIcon = WiFiIcon_Ap;
                char sbuf[MAX_SSID_LENGTH+1];
                if (CONFIG::read_string(EP_AP_SSID, sbuf, MAX_SSID_LENGTH))
                    return sbuf;
                else
                    return F("Can't get SSID");
            }

        default:
            return F("WiFi off");
    }
}

void StatusController::updateLeds()
{
    updateErrorLed();
    updateWiFiLed();
}

void StatusController::updateErrorLed()
{
    if (Board::pLedR == NULL) // Error LED
    {
        return;
    }

    if (Board::pVoltageMonitor != NULL &&
        Board::pVoltageMonitor->getStatus() != VMonStatus_Ok)
    {
        Board::pLedR->blink(250);
    }
    else if (WiFi.getMode() == WIFI_STA)
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

void StatusController::updateWiFiLed()
{
    if (Board::pLedB == NULL) // WiFi LED
    {
        return;
    }

    WiFiMode_t mode = WiFi.getMode();
    if (mode == WIFI_STA && WiFi.isConnected())
        Board::pLedB->on();
    else if (mode == WIFI_AP || mode == WIFI_AP_STA)
        Board::pLedB->blink(1500);
    else
        Board::pLedB->off();
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
    if (_updateTimer.milliSeconds() > 100)
    {
        // Do not update summary more often than once per 100 ms
        updateSummary();
        updateLeds();
        _updateTimer.restart();
    }
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

#ifdef PIN_ANALOG_VMON
    VoltageMonitor cVoltageMonitor(PIN_ANALOG_VMON, VMON_DIVIDER_RATIO);
    VoltageMonitor* const Board::pVoltageMonitor = &cVoltageMonitor;
#else
    VoltageMonitor* const Board::pVoltageMonitor = NULL;
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

    if (pVoltageMonitor != NULL)
    {
        bool succeded = true;

        int32_t correction = DEFAULT_VMON_CORRECTION_PPM;
        succeded &= CONFIG::read_buffer(EP_VMON_CORRECTION_PPM, (byte *)&correction, sizeof(int32_t));
        pVoltageMonitor->setCorrection_ppm(correction);

        int32_t targetVoltage = DEFAULT_VMON_TARGET_VOLTAGE_mV;
        succeded &= CONFIG::read_buffer(EP_VMON_TARGET_VOLTAGE_mV, (byte *)&targetVoltage, sizeof(int32_t));
        pVoltageMonitor->setTargetVoltage_mV(targetVoltage);

        uint8_t alarmThreshold = DEFAULT_VMON_ALARM_THRESHOLD;
        succeded &= CONFIG::read_byte(EP_VMON_ALARM_THRESHOLD, &alarmThreshold);
        pVoltageMonitor->setAlarmThreshold_percent(alarmThreshold);

        if (!succeded)
        {
            status.print(F("VMON cfg. error"));
        }
    }
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


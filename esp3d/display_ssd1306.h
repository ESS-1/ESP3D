/*
  display_ssd1306.h - SED1306 display driver.

  Copyright (c) 2018 Eugene Shelkovin. All rights reserved.
  This file is distributed under MIT license.
  MIT license may not be applied to the whole software or its files which
  are not explicitly marked as those distributed under MIT license.
*/

#pragma once

#include "display.h"
#include <Arduino.h>
#include <Wire.h>
#include "SSD1306Wire.h"

#include "timer.h"
#include "icons/off.h"
#include "icons/ap.h"
#include "icons/sta.h"

#define SSD1306_LINES            (5)
#define SSD1306_CHARS_PER_LINE   (36)    // includes NULL

#define SSD1306_BRIGHTNESS_HIGH  (0xF0)
#define SSD1306_BRIGHTNESS_LOW   (0x00)
#define SSD1306_DIMMING_DELAY    (10000) // ms
#define SSD1306_DIMMING_SLOWDOWN (35)    // ms per brightness unit

#define SSD1306_PRECHARGE        (0x11)
#define SSD1306_COM_DESELECT     (0x20)

class DisplaySSD1306 : public Display
{
private:
    SSD1306Wire _display;
    uint8_t _address;
    uint8_t _sda;
    uint8_t _scl;
    uint8_t _line_idx;
    WiFiIcon _icon;
    char _summary[SSD1306_CHARS_PER_LINE];
    char _log[SSD1306_LINES][SSD1306_CHARS_PER_LINE];

    uint8_t _brightness;
    Timer _dimmingTimer;

    void setBrightness(uint8_t brightness)
    {
        _display.setContrast(brightness, SSD1306_PRECHARGE, SSD1306_COM_DESELECT);

        _brightness = brightness;
        if (brightness >= SSD1306_BRIGHTNESS_HIGH)
        {
            _dimmingTimer.restart();
        }
    }

    void refresh()
    {
        _display.clear();

        // Draw the summmary area
        const uint8_t *pIcon = getIconData();
        int16_t statusStringX = 0;
        if (pIcon != NULL)
        {
            _display.drawXbm(0, 0, 12, 12, pIcon);
            statusStringX = 15;
        }

        _display.drawString(statusStringX, 0, _summary);
        _display.drawLine(0, 13, 127, 13);

        // Draw the log section
        int16_t y = 12;
        for (uint8_t i = 0; i<SSD1306_LINES; ++i, y+=10)
        {
            uint8_t idx = i+_line_idx;
            if (idx >= SSD1306_LINES)
            {
                idx-=SSD1306_LINES;
            }

            _display.drawString(0, y, _log[idx]);
        }

        _display.display();
        setBrightness(SSD1306_BRIGHTNESS_HIGH);
    }

    const uint8_t* getIconData()
    {
        switch (_icon)
        {
        case WiFiIcon_Off:
            return xbm::off_12;
        case WiFiIcon_Ap:
            return xbm::ap_12;
        case WiFiIcon_Sta:
            return xbm::sta_12;
        default:
            return NULL;
        }
    }

    bool updateLine(const char * src, char dst[SSD1306_CHARS_PER_LINE])
    {
        bool changed = false;
        bool skipNextIfPercent = false;

        for (uint16_t i = 0; i < SSD1306_CHARS_PER_LINE-1; ++i, ++src)
        {
            // Unescape '%%'
            if (skipNextIfPercent && *src == '%')
            {
                skipNextIfPercent = false;
                ++src;
            }
            skipNextIfPercent = (*src == '%');

            if (*src != dst[i])
            {
                dst[i] = *src;
                changed = true;
            }

            if (*src == NULL)
            {
                break;
            }
        }

        return changed;
    }

    void updateBrightness()
    {
        if (_brightness <= SSD1306_BRIGHTNESS_LOW)
        {
            return;
        }

        uint32_t dt = _dimmingTimer.milliSeconds();
        if (dt < (SSD1306_DIMMING_DELAY+SSD1306_DIMMING_SLOWDOWN))
        {
            return;
        }

        if (_brightness >= SSD1306_BRIGHTNESS_HIGH)
        {
            setBrightness(SSD1306_BRIGHTNESS_HIGH-1);
        }
        else
        {
            int32_t brightness = int32_t(SSD1306_BRIGHTNESS_HIGH) - (dt-SSD1306_DIMMING_DELAY)/SSD1306_DIMMING_SLOWDOWN;
            setBrightness(brightness < SSD1306_BRIGHTNESS_LOW
                ? SSD1306_BRIGHTNESS_LOW
                : brightness);
        }
    }

public:
    DisplaySSD1306(uint8_t address, uint8_t sda, uint8_t scl)
    : _display(address, sda, scl),
      _address(address),
      _scl(scl),
      _sda(sda),
      _line_idx(0),
      _brightness(0)
    {
    }

    virtual void init()
    {
        _display.init();
        _display.displayOff();
        _display.resetDisplay();
        _display.flipScreenVertically();

        setBrightness(SSD1306_BRIGHTNESS_LOW);

        // Configure SSD1306 clock: 0xD5 -> 0x80
        // Default value used by the library causes uneven pixel brightness
        Wire.beginTransmission(_address);
        Wire.write(0x80);
        Wire.write(0xD5);
        Wire.write(0x80);
        Wire.write(0x80);
        Wire.endTransmission();
//TODO:        _display.setClockDiv(0x80);

        _display.displayOn();

        // Simple display test pattern
        _display.fillRect(0, 0, 128, 64);
        _display.display();
    }

    virtual bool isPinUsed(uint8_t pin) const
    {
        return pin == _scl || pin == _sda;
    }

    virtual void update()
    {
        updateBrightness();
    }

    virtual void printSummary(WiFiIcon icon, const char *s)
    {
        register bool hasChanged = (_icon != icon);
        _icon = icon;

        hasChanged |= updateLine(s, _summary);
        if (hasChanged)
            refresh();
    }

    virtual void print(const char *s)
    {
        if (updateLine(s, _log[_line_idx]))
        {
            refresh();
        }
    }

    virtual void newLine()
    {
        if (_line_idx == 0)
        {
            _line_idx = SSD1306_LINES-1;
        }
        else
        {
            --_line_idx;
        }
    }
};

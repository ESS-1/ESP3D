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

#include "icons/off.h"
#include "icons/ap.h"
#include "icons/sta.h"

#define SSD1306_LINES (5)
#define SSD1306_CHARS_PER_LINE (48) // includes NULL

class DisplaySSD1306 : public Display
{
private:
    SSD1306Wire _display;
    uint8_t _sda;
    uint8_t _scl;
    int _line_idx;
    WiFiIcon _icon;
    char _summary[SSD1306_CHARS_PER_LINE];
    char _log[SSD1306_LINES][SSD1306_CHARS_PER_LINE];

    void refresh()
    {
        _display.clear();

        // Draw the summmary area
        const uint8_t *pIcon = getIconData();
        int statusStringX = 0;
        if (pIcon != NULL)
        {
            _display.drawXbm(0, 0, 12, 12, pIcon);
            statusStringX = 15;
        }

        _display.drawString(statusStringX, 0, _summary);
        _display.drawLine(0, 13, 127, 13);

        // Draw the log section
        uint8_t y = 12;
        for (int i = 0; i<SSD1306_LINES; ++i, y+=10)
        {
            int idx = i+_line_idx;
            if (idx >= SSD1306_LINES)
            {
                idx-=SSD1306_LINES;
            }

            _display.drawString(0, y, _log[idx]);
        }

        _display.display();
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

public:
    DisplaySSD1306(uint8_t address, uint8_t sda, uint8_t scl)
    : _display(address, sda, scl),
      _scl(scl),
      _sda(sda),
      _line_idx(0)
    {
    }

    virtual void init()
    {
        _display.init();
        _display.resetDisplay();
        _display.flipScreenVertically();
        _display.displayOn();
    }

    virtual bool isPinUsed(uint8_t pin) const
    {
        return pin == _scl || pin == _sda;
    }

    virtual void printSummary(WiFiIcon icon, const char *s)
    {
        register bool hasChanged = (_icon != icon);
        _icon = icon;

        for (int i = 0; i < SSD1306_CHARS_PER_LINE-1; ++i)
        {
            if (s[i] != _summary[i])
            {
                _summary[i] = s[i];
                hasChanged = true;
            }

            if (s[i] == NULL)
            {
                break;
            }
        }

        if (hasChanged)
            refresh();
    }

    virtual void print(const char *s)
    {
        bool skipNextIfPercent = false;
        for (int i = 0; i < SSD1306_CHARS_PER_LINE-1; ++i)
        {
            register char c = s[i];

            // Unescape '%%'
            if (skipNextIfPercent && c == '%')
            {
                skipNextIfPercent = false;
                continue;
            }
            skipNextIfPercent = (c == '%');

            _log[_line_idx][i] = c;
            if (c == NULL)
            {
                break;
            }
        }

        refresh();
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

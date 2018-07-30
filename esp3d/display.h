/*
  display.h - ESP3D board display abstraction.

  Copyright (c) 2018 Eugene Shelkovin. All rights reserved.
  This file is distributed under MIT license.
  MIT license may not be applied to the whole software or its files which
  are not explicitly marked as those distributed under MIT license.
*/

#pragma once

enum WiFiIcon
{
    WiFiIcon_None,
    WiFiIcon_Off,
    WiFiIcon_Ap,
    WiFiIcon_Sta
};

class Display
{
public:
    virtual void init() = 0;
    virtual bool isPinUsed(uint8_t pin) const = 0;
    virtual void printSummary(WiFiIcon icon, const char *s) = 0;
    virtual void print(const char *s) = 0;
    virtual void newLine() = 0;

    inline void printSummary(WiFiIcon icon, const String& s)
    {
        printSummary(icon, s.c_str());
    }
    inline void print(const String& s)
    {
        print(s.c_str());
    }
};


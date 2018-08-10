/*
  timer.h - simple timer with 1 ms resolution (based on millis function)

  Copyright (c) 2018 Eugene Shelkovin. All rights reserved.
  This file is distributed under MIT license.
  MIT license may not be applied to the whole software or its files which
  are not explicitly marked as those distributed under MIT license.
*/

#pragma once

#include <Arduino.h>


class Timer
{
protected:
    uint32_t _start_ms = 0;

    inline uint32_t getTimeDelta(uint32_t t0, uint32_t t) const
    {
        return t >= t0 ? t-t0 : 1+~t0+t;
    };

public:
    void restart()
    {
        _start_ms = millis();
    }

    uint32_t milliSeconds()
    {
        return getTimeDelta(_start_ms, millis());
    }

    void adjustStart(int32_t msToAdd)
    {
        _start_ms += msToAdd;
    }
};

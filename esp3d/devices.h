/*
  devices.h - esp3d hardware abstraction

  Copyright (c) 2018 Eugene Shelkovin. All rights reserved.
  This file is distributed under MIT license.
  MIT license may not be applied to the whole software or its files which
  are not explicitly marked as those distributed under MIT license.
*/

#pragma once

#include <Arduino.h>

// GpioDevice
class GpioDevice
{
protected:
    uint8_t _pin;
    uint8_t _active;

    GpioDevice(uint8_t pin, uint8_t active)
    : _pin(pin),
      _active(active)
    {
    }

    inline uint32_t getTimeDelta(uint32_t t0, uint32_t t) const
    {
        return t >= t0 ? t-t0 : 1+~t0+t;
    };

public:
    uint8_t getPin() const;
};


// GpioOutputDevice
class GpioOutputDevice : public GpioDevice
{
private:
    enum : uint8_t
    {
        mode_const,
        mode_blink,
        mode_pulse
    } _mode;

    uint16_t _tOn_ms;
    uint16_t _tOff_ms;
    uint32_t _startTime_ms;

public:
    GpioOutputDevice(uint8_t pin, uint8_t active = LOW);

    void on();
    void off();
    bool isOn() const;
    void blink(uint16_t tCycle);
    void blink(uint16_t tOn_ms, uint16_t tOff_ms);
    void pulse(uint16_t tPulse_ms);

    void update();
};


// SimpleGpioOutputDevice
class SimpleGpioOutputDevice : public GpioDevice
{
public:
    SimpleGpioOutputDevice(uint8_t pin, uint8_t active = LOW);

    void on();
    void off();
    bool isOn() const;
};


// HoldButton
class HoldButton : public GpioDevice
{
private:
    void (*_handler)();
    uint16_t _minHoldTime_ms;
    volatile uint32_t* _pInterruptTime;

public:
    struct IsrDef
    {
        volatile uint32_t interruptTime;
        uint8_t pin;
        void (*isr)();
    };

public:
    HoldButton(IsrDef &isrDef, void (*handler)(), uint16_t minHoldTime_ms, uint8_t active = LOW);
    virtual void update();
};

#define HOLDBUTTON_ISR(pin) (HoldButton_IsrDef_##pin)
#define HOLDBUTTON_DECLARE_ISR(pin) HoldButton::IsrDef HoldButton_IsrDef_##pin\
    = {0, pin, []{HoldButton_IsrDef_##pin.interruptTime = millis();}};


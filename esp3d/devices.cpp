/*
  devices.cpp - esp3d hardware abstraction

  Copyright (c) 2018 Eugene Shelkovin. All rights reserved.
  This file is distributed under MIT license.
  MIT license may not be applied to the whole software or its files which
  are not explicitly marked as those distributed under MIT license.
*/

#include "devices.h"


// GpioOutputDevice
GpioOutputDevice::GpioOutputDevice(uint8_t pin, uint8_t active /*=LOW*/)
    : GpioDevice(pin, active),
      _mode(mode_const),
      _tOn_ms(0),
      _tOff_ms(0)
{
    digitalWrite(_pin, !_active);
    pinMode(_pin, OUTPUT);
}

void GpioOutputDevice::on()
{
    _mode = mode_const;
    digitalWrite(_pin, _active);
}

void GpioOutputDevice::off()
{
    _mode = mode_const;
    digitalWrite(_pin, !_active);
}

bool GpioOutputDevice::isOn() const
{
    return (digitalRead(_pin) != 0) == (_active != 0);
}

void GpioOutputDevice::blink(uint16_t tCycle)
{
    uint16_t tOn = tCycle >> 1;
    blink(tOn, tCycle - tOn);
}

void GpioOutputDevice::blink(uint16_t tOn_ms, uint16_t tOff_ms)
{
    if (_mode == mode_blink &&
        _tOn_ms == tOn_ms &&
        _tOff_ms == tOff_ms)
    {
        return;
    }
	
    _mode = mode_blink;
    _tOn_ms = tOn_ms;
    _tOff_ms = tOff_ms;

    _timer.restart();
    digitalWrite(_pin, _active);
}

void GpioOutputDevice::pulse(uint16_t tPulse_ms)
{
    if (_mode == mode_pulse)
    {
        _tOn_ms += tPulse_ms;
        return;
    }

    _mode = mode_pulse;
    _tOn_ms = tPulse_ms;

    _timer.restart();
    digitalWrite(_pin, _active);
}

void GpioOutputDevice::update()
{
    if (_mode != mode_const)
    {
        uint32_t dt = _timer.milliSeconds();

        if (_mode == mode_blink)
        {
            uint32_t tCycle = _tOn_ms+_tOff_ms;
            uint32_t remainder = dt%tCycle;
            digitalWrite(_pin, remainder < _tOn_ms ? _active : !_active);

            // Increase start time to avoid possible overflow
            if (dt > remainder)
            {
                _timer.adjustStart(dt-remainder);
            }
        }
        else if (dt >= _tOn_ms) // Single pulse
        {
            off();
        }
    }
}


// SimpleGpioOutputDevice
SimpleGpioOutputDevice::SimpleGpioOutputDevice(uint8_t pin, uint8_t active /*=LOW*/)
    : GpioDevice(pin, active)
{
    digitalWrite(_pin, !_active);
    pinMode(_pin, OUTPUT);
}

void SimpleGpioOutputDevice::on()
{
    digitalWrite(_pin, _active);
}

void SimpleGpioOutputDevice::off()
{
    digitalWrite(_pin, !_active);
}

bool SimpleGpioOutputDevice::isOn() const
{
    return (digitalRead(_pin) != 0) == (_active != 0);
}


// HoldButton
HoldButton::HoldButton(IsrDef &isrDef, void (*handler)(), uint16_t minHoldTime_ms, uint8_t active /*=LOW*/)
    : GpioDevice(isrDef.pin, active),
      _handler(handler),
      _minHoldTime_ms(minHoldTime_ms),
      _pTimer(&isrDef.timer)
{
    pinMode(_pin, INPUT);

    (*isrDef.isr)(); // Call ISR once to update the timer
    attachInterrupt(digitalPinToInterrupt(_pin), isrDef.isr, CHANGE);
}

bool HoldButton::isPressed()
{
    return (digitalRead(_pin) != 0) == (_active != 0);
}

void HoldButton::update()
{
    noInterrupts();
    uint32_t dt = _pTimer->milliSeconds();
    interrupts();

    if (isPressed() && dt >= _minHoldTime_ms)
    {
        _handler();

        noInterrupts();
        _pTimer->restart();
        interrupts();
    }
}


/*
  VoltageMonitor.cpp - ESP8266 voltage monitor

  Copyright (c) 2018 Eugene Shelkovin. All rights reserved.
  This file is distributed under MIT license.
  MIT license may not be applied to the whole software or its files which
  are not explicitly marked as those distributed under MIT license.
*/

#include "VoltageMonitor.h"


// VoltageMonitor
VoltageMonitor::VoltageMonitor(uint8_t pin, int8_t input_divider_ratio)
    : _pin(pin),
      _input_divider_ratio(input_divider_ratio),
      _target_uV(0),
      _calibration_ppm(1000000),
      _alarm_threshold_uV(0),
      _alarm_threshold_percent(0)
{
}

void VoltageMonitor::setCorrection_ppm(int32_t correction)
{
    _calibration_ppm = 1000000+correction;
}

void VoltageMonitor::setTargetVoltage_mV(int32_t targetVoltage)
{
    _target_uV = targetVoltage*1000;
    setAlarmThreshold_percent(_alarm_threshold_percent);
}

void VoltageMonitor::setAlarmThreshold_percent(uint8_t threshold)
{
    _alarm_threshold_percent = threshold;
    _alarm_threshold_uV = _target_uV*int32_t(threshold)/100;
}

int32_t VoltageMonitor::getVoltage_uV()
{
    return ((analogRead(_pin)*_calibration_ppm)+512)/1024*_input_divider_ratio;
}

int32_t VoltageMonitor::getVoltage_mV()
{
    return (getVoltage_uV()+500)/1000;
}

VoltageMonitorStatus VoltageMonitor::getStatus()
{
    if (_alarm_threshold_uV < 1)
        return VMonStatus_Ok;

    int32_t dv = getVoltage_uV()-_target_uV;
    if (dv > _alarm_threshold_uV)
    {
        return VMonStatus_Overvoltage;
    }
    if (dv < -_alarm_threshold_uV)
    {
        return VMonStatus_Undervoltage;
    }

    return VMonStatus_Ok;
}

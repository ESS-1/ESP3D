/*
  VoltageMonitor.h - ESP8266 voltage monitor

  Copyright (c) 2018 Eugene Shelkovin. All rights reserved.
  This file is distributed under MIT license.
  MIT license may not be applied to the whole software or its files which
  are not explicitly marked as those distributed under MIT license.
*/

#pragma once

#include <Arduino.h>


enum VoltageMonitorStatus
{
    VMonStatus_Ok,
    VMonStatus_Undervoltage,
    VMonStatus_Overvoltage
};

// VoltageMonitor
class VoltageMonitor
{
private:
    uint8_t _pin;
    int8_t _input_divider_ratio;
    uint8_t _alarm_threshold_percent;
    int32_t _target_uV;
    int32_t _calibration_ppm;
    int32_t _alarm_threshold_uV;

public:
    VoltageMonitor(uint8_t pin, int8_t input_divider_ratio);
    void setCorrection_ppm(int32_t correction);
    void setTargetVoltage_mV(int32_t targetVoltage);
    void setAlarmThreshold_percent(uint8_t threshold);
    int32_t getVoltage_uV();
    int32_t getVoltage_mV();
    String formatVoltage();
    VoltageMonitorStatus getStatus();
};

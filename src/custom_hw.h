// Copyright 2022 Benjamin Vedder <benjamin@vedder.se>
// Copyright 2024 Lukas Hrazky
//
// This file is part of the Refloat VESC package.
//
// Refloat VESC package is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// Refloat VESC package is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "conf/datatypes.h"
#include <stdbool.h>
#include <stdint.h>
#define CUSTOM_HW_VERSION_MAJOR 3
#define CUSTOM_HW_VERSION_MINOR 0
#define ESP32_COMMAND_ID 102

// 燈光控制狀態結構體
typedef struct {
    float last_light_toggle_time;
    float last_brake_flash_time;
    float last_idle_time;
    CUSTOM_COB_LIGHT_MODE light_mode;
    bool headlight_state;
    bool brakelight_state;
    float light_toggle_interval;  // 頭燈和煞車燈的交替時間
    float idle_interval;  // 閒置時間,計時器假如10分鐘,會減一分鐘, 直到最後為1分鐘蜂鳴器響一次
} CustomLightControl;

void ext_dcdc_init();

void custom_lights_control_init(CustomLightControl *state, CUSTOM_COB_LIGHT_MODE light_mode, CUSTOM_IDLE_TIME mode);

void custom_lights_running(
    CustomLightControl *light_state,
    CUSTOM_IDLE_TIME mode,
    float abs_erpm,
    float pid_value,
    float system_time
);

bool custom_lights_idle(CustomLightControl *light_state, float system_time);

void ext_dcdc_enable( bool enable );
void beeper_init();
void ext_beeper_on();
void ext_beeper_off();
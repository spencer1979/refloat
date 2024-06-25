#pragma once
#ifndef CUSTOM_HW_H
#define CUSTOM_HW_H

#include <stdbool.h>
#include <stdint.h>
#include "conf/datatypes.h"
#include "st_types.h"
#include "state.h"
#define USE_CUSTOM_HW  // 設為1可控制外部DCDC及引擎聲音校及COB LED 
#define CUSTOM_HW_VERSION_MAJOR 3
#define CUSTOM_HW_VERSION_MINOR 3
#define ESP32_COMMAND_ID 102
#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))

/**Head light blink var */
#define SEC_TO_MILLS 1000
#define LIGHT_BLINK_TIME_MIN 300.00	 // blink 0.3 seconds
#define LIGHT_BLINK_TIME_MAX 1500.00	 // blink 1.5 seconds
#define BRK_LIGHT_BLINK_TIME 150.00 // 150 ms rear light blink when brake  .
/**Lights GPIO  */
#if (CUSTOM_HW_VERSION_MAJOR <=2)
#define REAR_LIGHT_GPIO GPIOB
#define REAR_LIGHT_PIN 7
#define FWD_LIGHT_GPIO GPIOB
#define FWD_LIGHT_PIN 5
#else
#define FWD_LIGHT_GPIO GPIOC
#define FWD_LIGHT_PIN 5
#define REAR_LIGHT_GPIO GPIOB
#define REAR_LIGHT_PIN 5
#endif
/**Light off /on macro  */
#define FWD_LIGHT_ON() VESC_IF->set_pad(FWD_LIGHT_GPIO, FWD_LIGHT_PIN)
#define FWD_LIGHT_OFF() VESC_IF->clear_pad(FWD_LIGHT_GPIO, FWD_LIGHT_PIN)
#define REAR_LIGHT_ON() VESC_IF->set_pad(REAR_LIGHT_GPIO, REAR_LIGHT_PIN)
#define REAR_LIGHT_OFF() VESC_IF->clear_pad(REAR_LIGHT_GPIO, REAR_LIGHT_PIN)
/**FAN Control GPIO */
// #define FAN_GPIO		GPIOC
// #define FAN_PIN			12

/** external dcdc control gpio */
#define EXT_DCDC_GPIO GPIOD
#define EXT_DCDC_PIN 2
#define EXT_DCDC_ON() VESC_IF->set_pad(EXT_DCDC_GPIO, EXT_DCDC_PIN)
#define EXT_DCDC_OFF() VESC_IF->clear_pad(EXT_DCDC_GPIO, EXT_DCDC_PIN)



//燈光控制狀態結構體
typedef struct {
    float last_toggle_time;
    float last_brake_flash_time;
    CUSTOM_COB_LIGHT_MODE light_mode;
    bool headlight_state;
    bool brakelight_state;
    float toggle_interval; //頭燈和煞車燈的交替時間
    float flash_interval;//煞車燈的閃爍時間
} CustomLightControl;

//初始化燈光控制系統
void custom_lights_control_init(CustomLightControl *state,CUSTOM_COB_LIGHT_MODE light_mode);

//燈光控制函數
void control_lights(
    CustomLightControl *light_state,
    State  *state,
    float abs_erpm,
    float pid_value,
    float system_time
);

//前燈後燈控制函數
void set_custom_headlight(bool state);
void set_custom_brakelight(bool state);

#endif // CUSTOM_HW_H

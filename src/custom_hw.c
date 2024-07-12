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
#include "custom_hw.h"
#include <math.h>
#include <string.h>
#include "st_types.h"
#include "vesc_c_if.h"


/**FAN Control GPIO */
// #define FAN_GPIO		GPIOC
// #define FAN_PIN			12

/** external dcdc control gpio */
#define EXT_DCDC_GPIO GPIOD
#define EXT_DCDC_PIN 2
#define EXT_DCDC_ON() VESC_IF->set_pad(EXT_DCDC_GPIO, EXT_DCDC_PIN)
#define EXT_DCDC_OFF() VESC_IF->clear_pad(EXT_DCDC_GPIO, EXT_DCDC_PIN)

/**Lights gpio */
#if (CUSTOM_HW_VERSION_MAJOR <= 2)
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

/**Head light blink  default */
#define SEC_TO_MILLS 1000
#define LIGHT_BLINK_TIME_MIN 250.00	 // 最小前後燈交替時間
#define LIGHT_BLINK_TIME_MAX 1500.00	 // 最大前後燈交替時間 , 交替時間隨著轉變快.
#define BRK_LIGHT_BLINK_TIME 150.00 // 煞車時閃爍時間


#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))

/**FAN Control GPIO */
// #define FAN_GPIO		GPIOC
// #define FAN_PIN			12

//Buzzer gpio
#if (CUSTOM_HW_VERSION_MINOR >= 3 && CUSTOM_HW_VERSION_MAJOR >= 3 )
    #define BEEPER_GPIO_PORT  GPIOC
    #define BEEPER_GPIO_PIN 13
  #else
    #define BEEPER_GPIO_PORT GPIOB
    #define BEEPER_GPIO_PIN 9
  #endif
#define EXT_BEEPER_ON() VESC_IF->set_pad(BEEPER_GPIO_PORT, BEEPER_GPIO_PIN)
#define EXT_BEEPER_OFF() VESC_IF->clear_pad(BEEPER_GPIO_PORT, BEEPER_GPIO_PIN)

// Function to map a value from one range to another with clamping.
// Parameters:
//   x - The value to map.
//   in_min - The lower bound of the input range.
//   in_max - The upper bound of the input range.
//   out_min - The lower bound of the output range.
//   out_max - The upper bound of the output range.
// Returns:
//   The mapped value in the output range.

float map_value(float x, float in_min, float in_max, float out_min, float out_max) {
    // Prevent division by zero
    if (in_min == in_max) {
        return out_min; // Or handle this case in some other way
    }

    // Clamp the input value to the input range
    if (x < in_min) {
        x = in_min;
    } else if (x > in_max) {
        x = in_max;
    }

    // Perform the mapping
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


static float get_idle_warning_timer(CUSTOM_IDLE_TIME mode) {
    switch (mode) {
    case IDLE_WARNING_TIME_DISABLE:
        return 0.0f;
    case IDLE_WARNING_TIME_1M:
        return 60.0f;
    case IDLE_WARNING_TIME_5M:
        return 300.0f;
    case IDLE_WARNING_TIME_10M:
        return 600.0f;
    case IDLE_WARNING_TIME_30M:
        return 1800.0f;
    case IDLE_WARNING_TIME_60M:
        return 3600.0f;
    case IDLE_WARNING_TIME_120M:
        return 7200.0f;
    default:
        return 0.0f;
    }
}

static void set_custom_headlight(bool state) {
    if (state) {
       FWD_LIGHT_ON();
    } else {
       FWD_LIGHT_OFF();
    }
}

static void set_custom_brakelight(bool state) {
    if (state) {
        REAR_LIGHT_ON();
    } else {
         REAR_LIGHT_OFF();
    }
}

    

void ext_dcdc_init()
{
      VESC_IF->set_pad_mode(
        EXT_DCDC_GPIO,
        EXT_DCDC_PIN,
        PAL_STM32_MODE_OUTPUT | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL
    );
}

void custom_lights_init(
    CustomLightControl *state, CUSTOM_COB_LIGHT_MODE light_mode , CUSTOM_IDLE_TIME mode
) {
    state->last_light_toggle_time = 0;
    state->last_brake_flash_time = 0;
    state->light_mode = light_mode;
    state->toggle_light_state = false;
    state->brake_light_state = false;
    state->light_toggle_interval = LIGHT_BLINK_TIME_MAX;
    state->idle_interval=get_idle_warning_timer( mode);

      VESC_IF->set_pad_mode(
        FWD_LIGHT_GPIO,
        FWD_LIGHT_PIN,
        PAL_STM32_MODE_OUTPUT | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL
    );
    VESC_IF->set_pad_mode(
        REAR_LIGHT_GPIO,
        REAR_LIGHT_PIN,
        PAL_STM32_MODE_OUTPUT | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL
    );
        //只有燈全亮時,其他狀態都是熄滅
    if (light_mode == COB_LIGHT_FULL_ON) {
        FWD_LIGHT_ON();
        REAR_LIGHT_ON();
    } else {
        FWD_LIGHT_OFF();
        REAR_LIGHT_OFF();
    }
}

void custom_lights_running(
    CustomLightControl *light_state,
    float abs_erpm,
    float pid_value,
    float system_time
) { 
  static float previous_pid_value =0;

      if (light_state->last_light_toggle_time ==0) {  // 檢查交替閃爍時間 是不是0 , 如為0記住系統時間
                    light_state->last_light_toggle_time = system_time;
                }
                    // 如果煞車，依照設定的閃爍時間間隔閃爍煞車燈 ,
    if (light_state->last_brake_flash_time ==0) {  // 檢查煞車閃爍時間 是不是0 , 如為0記住系統時間
                light_state->last_brake_flash_time = system_time;
            }

    if (abs_erpm >= 100 ) // 馬達轉動 
    {   

        if (pid_value > -6) {

            switch (light_state->light_mode) {
            case COB_LIGHT_OFF:
                set_custom_headlight(false);
                set_custom_brakelight(false);
                break;
            case COB_LIGHT_FLASH:
                if ((system_time - light_state->last_light_toggle_time) * SEC_TO_MILLS >= light_state->light_toggle_interval) {  // 判斷計時器是否時間到{
                    light_state->toggle_light_state = !light_state->toggle_light_state;
                    set_custom_headlight(light_state->toggle_light_state);
                    set_custom_brakelight(!light_state->toggle_light_state);
                    light_state->light_toggle_interval = map_value(abs_erpm, 500.00, 2500.00, LIGHT_BLINK_TIME_MAX, LIGHT_BLINK_TIME_MIN);  // 映射abs_erpm 為閃爍時間
                    light_state->last_light_toggle_time = system_time;  // 重設頭燈閃爍計時器  
                }
                break;
            case COB_LIGHT_FULL_ON:
                set_custom_headlight(true);
                set_custom_brakelight(true);
                break;
            }
        } else {
            //急煞時會有一個大的負的pid_value 
           if ( fabsf(pid_value)-fabsf(previous_pid_value ) > 10 )
            {
                     if ((system_time - light_state->last_brake_flash_time)  * SEC_TO_MILLS  >= BRK_LIGHT_BLINK_TIME )
                      {
                         light_state->brake_light_state = !light_state->brake_light_state;
                         set_custom_brakelight(light_state->brake_light_state);
                         // 重置煞車燈計時器
                         light_state->last_brake_flash_time=system_time;
                      
                      }  
                        previous_pid_value=pid_value;
          
           }
            // if erpm >0 is forward Brake , erpm < 0 if reverse brake      
        }

    }
}

void custom_lights_idle(CUSTOM_COB_LIGHT_MODE mode ) {

    if (mode == COB_LIGHT_FULL_ON) {
        set_custom_headlight(true);
        set_custom_brakelight(true);

    } else{
        set_custom_headlight(false);
        set_custom_brakelight(false);
    }
}

void custom_lights_off( ) {
        set_custom_headlight(false);
        set_custom_brakelight(false);
}
void ext_dcdc_enable( bool enable  )
{
   if (enable) {
        EXT_DCDC_ON();
    } else {
        EXT_DCDC_OFF();
    }
}


void beeper_init()
 {
    VESC_IF->set_pad_mode(
        BEEPER_GPIO_PORT,
        BEEPER_GPIO_PIN,
        PAL_STM32_MODE_OUTPUT | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL
    );
}

void ext_beeper_on()
{
   EXT_BEEPER_ON();

}
void ext_beeper_off(){
    EXT_BEEPER_OFF();

}
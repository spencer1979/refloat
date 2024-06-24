#pragma once

#ifndef CUSTOM_HW_H
#define CUSTOM_HW_H
#include "conf/datatypes.h"
#include "motor_data.h"
#include "state.h"
#define USE_CUSTOM_HW  // 設為1可控制外部DCDC及引擎聲音校及COB LED 
#define CUSTOM_HW_VERSION_MAJOR 3
#define CUSTOM_HW_VERSION_MINOR 3
#define ESP32_COMMAND_ID 102
#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))
void custom_hw_init(void );

typedef enum
{	ESP_COMMAND_GET_READY=0,
	ESP_COMMAND_GET_ADV_INFO ,  // get ADVANCED setting only for SPESC
	ESP_COMMAND_ENGINE_SOUND_INFO, // engine sound info , erpm ,duty
	// esp32 get the sound trigger
	ESP_COMMAND_SOUND_GET,
	// app set sound trigger
	ESP_COMMAND_SOUND_SET,
	// enable item data
	ESP_COMMAND_ENABLE_ITEM_INFO,

} esp_commands;

typedef enum
{
	SOUND_HORN_TRIGGERED=0 ,
	SOUND_EXCUSE_ME_TRIGGERED,
	SOUND_POLICE_TRIGGERED,
	SOUND_DANGER_TRIGGERED
} sound_triggered_mask;


//Determine the function of a certain bit
typedef enum
{
EXT_DCDC_ENABLE_MASK_BIT=0,
ENGINE_SOUND_ENABLE_MASK_BIT,
START_UP_WARNING_ENABLE_MASK_BIT,
} float_enable_mask;



/**Head light blink var */
#define SEC_TO_MILLS 1000
#define LIGHT_BLINK_TIME_MIN 300	 // blink 0.3 seconds
#define LIGHT_BLINK_TIME_MAX 1500	 // blink 1.5 seconds
#define BRK_LIGHT_BLINK_TIME_MIN 150 // 150 ms rear light blink when brake  .
/**Lights GPIO  */
#if (HW_VERSION_MAJOR <=2)
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




#endif
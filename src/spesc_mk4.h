// spesc_mk4.h
#pragma once

/** External DCDC control GPIO for hardware version MK3 (version 4) */
#define EXT_DCDC_GPIO GPIOC
#define EXT_DCDC_PIN 12

/** Lights GPIO for hardware version MK3 (version 4) */
#define FWD_LIGHT_GPIO GPIOC
#define FWD_LIGHT_PIN 5
#define REAR_LIGHT_GPIO GPIOB
#define REAR_LIGHT_PIN 5

/** Buzzer GPIO for hardware version MK3 (version 4) */
#if( CUSTOM_HW_VERSION_MINOR >=5 )
#define BEEPER_GPIO_PORT GPIOB // origin Servo pin 
#define BEEPER_GPIO_PIN 6
#else
#define BEEPER_GPIO_PORT GPIOB
#define BEEPER_GPIO_PIN 12
#endif
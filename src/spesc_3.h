#pragma once

/** External DCDC control GPIO for hardware version 3 */
#define EXT_DCDC_GPIO GPIOD
#define EXT_DCDC_PIN 2

/** Lights GPIO for hardware version 3 */
#define FWD_LIGHT_GPIO GPIOC
#define FWD_LIGHT_PIN 5
#define REAR_LIGHT_GPIO GPIOB
#define REAR_LIGHT_PIN 5

/** Buzzer GPIO for hardware version 3 */
#if CUSTOM_HW_VERSION_MINOR == 3
    #define BEEPER_GPIO_PORT GPIOC
    #define BEEPER_GPIO_PIN 13
#else
    #define BEEPER_GPIO_PORT GPIOB
    #define BEEPER_GPIO_PIN 9
#endif
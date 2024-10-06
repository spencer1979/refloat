#pragma once

#include "conf/datatypes.h"
#include "st_types.h"
#include "vesc_c_if.h"

// Define the hardware version here
#define CUSTOM_HW_VERSION_MAJOR 4  // Change this to 4 for MK3 version
#define CUSTOM_HW_VERSION_MINOR 5

// Include the appropriate hardware configuration header
#if CUSTOM_HW_VERSION_MAJOR == 3
    #include "spesc_3.h"
#elif CUSTOM_HW_VERSION_MAJOR == 4
    #include "spesc_mk4.h"
#else
    #error "Unsupported hardware version!"
#endif

// General definitions for both versions

// External DCDC control GPIO
#define EXT_DCDC_ON() VESC_IF->set_pad(EXT_DCDC_GPIO, EXT_DCDC_PIN)
#define EXT_DCDC_OFF() VESC_IF->clear_pad(EXT_DCDC_GPIO, EXT_DCDC_PIN)

// Light control macros
#define FWD_LIGHT_ON() VESC_IF->set_pad(FWD_LIGHT_GPIO, FWD_LIGHT_PIN)
#define FWD_LIGHT_OFF() VESC_IF->clear_pad(FWD_LIGHT_GPIO, FWD_LIGHT_PIN)
#define REAR_LIGHT_ON() VESC_IF->set_pad(REAR_LIGHT_GPIO, REAR_LIGHT_PIN)
#define REAR_LIGHT_OFF() VESC_IF->clear_pad(REAR_LIGHT_GPIO, REAR_LIGHT_PIN)

// Buzzer control macros
#define EXT_BEEPER_ON() VESC_IF->set_pad(BEEPER_GPIO_PORT, BEEPER_GPIO_PIN)
#define EXT_BEEPER_OFF() VESC_IF->clear_pad(BEEPER_GPIO_PORT, BEEPER_GPIO_PIN)

/**Head light blink  default */
#define SEC_TO_MILLS 1000
#define LIGHT_BLINK_TIME_MIN 200.00	 // 最小前後燈交替時間
#define LIGHT_BLINK_TIME_MAX 1200.00	 // 最大前後燈交替時間 , 交替時間隨著轉變快.
#define BRK_LIGHT_BLINK_TIME 100.00 // 煞車時閃爍時間
#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))

// Custom light control structure
typedef struct {
    float last_light_toggle_time;
    float last_brake_flash_time;
    bool toggle_light_state;
    bool brake_light_state;
    float light_toggle_interval;  // Headlight and brake light toggle interval
    bool is_idle_too_long;
} CustomLightControl;

// Function declarations
void ext_dcdc_init();
void beeper_init();
void custom_lights_init(CustomLightControl *state, CUSTOM_COB_LIGHT_MODE light_mode);
void custom_lights_running(CustomLightControl *light_state, CUSTOM_COB_LIGHT_MODE light_mode, float abs_erpm, float pid_value, float system_time);
void custom_lights_idle(CUSTOM_COB_LIGHT_MODE mode);
void custom_lights_off();
void ext_dcdc_enable(bool enable);
void ext_beeper_on();
void ext_beeper_off();

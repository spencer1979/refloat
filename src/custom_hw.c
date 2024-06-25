#include "custom_hw.h"
#include "vesc_c_if.h"

float utils_map(float x, float in_min, float in_max, float out_min, float out_max) {
    if (x < in_min) {
        return out_min;
    } else if (x > in_max) {
        return out_max;
    }
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void custom_lights_control_init(CustomLightControl *state, CUSTOM_COB_LIGHT_MODE light_mode) {
    state->last_toggle_time = 0;
    state->last_brake_flash_time = 0;
    state->light_mode = light_mode;
    state->headlight_state = false;
    state->brakelight_state = false;
    state->toggle_interval = LIGHT_BLINK_TIME_MAX;
    state->flash_interval = BRK_LIGHT_BLINK_TIME;
    // 初始化gpio
    // VESC_IF->set_pad_mode(
    //     FWD_LIGHT_GPIO,
    //     FWD_LIGHT_PIN,
    //     PAL_STM32_MODE_OUTPUT | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL
    // );
    // VESC_IF->set_pad_mode(
    //     REAR_LIGHT_GPIO,
    //     REAR_LIGHT_PIN,
    //     PAL_STM32_MODE_OUTPUT | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL
    // );
    // VESC_IF->set_pad_mode(
    //     EXT_DCDC_GPIO,
    //     EXT_DCDC_PIN,
    //     PAL_STM32_MODE_OUTPUT | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL
    // );
}

float get_idle_warning_timer(CUSTOM_IDLE_TIME mode) {
    switch (mode) {
    case IDLE_WARNING_TIME_DISABLE:
        return 0;
    case IDLE_WARNING_TIME_1M:
        return 60;
    case IDLE_WARNING_TIME_5M:
        return 300;
    case IDLE_WARNING_TIME_10M:
        return 600;
    case IDLE_WARNING_TIME_30M:
        return 1800;
    case IDLE_WARNING_TIME_60M:
        return 3600;
    case IDLE_WARNING_TIME_120M:
        return 7200;
    default:
        return 0;
    }
}

void set_custom_headlight(bool state) {
    if (state) {
        // GPIOB_PIN6 = HIGH;
    } else {
        // GPIOB_PIN6 = LOW;
    }
}

void set_custom_brakelight(bool state) {
    if (state) {
        // GPIOB_PIN7 = HIGH;
    } else {
        // GPIOB_PIN7 = LOW;
    }
}

void control_lights(
    CustomLightControl *light_state,
    State  *state,
    float abs_erpm,
    float pid_value,
    float system_time
) {
    // 如果马达未转动，关闭头灯和刹车灯
    if (abs_erpm >= 20 && state->state == STATE_RUNNING) {

        if (pid_value > -6) {
            if (light_state->light_mode == COB_LIGHT_OFF) {

                set_custom_headlight(false);
                set_custom_brakelight(false);
            } else if (light_state->light_mode == COB_LIGHT_FLASH) {

                if ((system_time - light_state->last_toggle_time) * SEC_TO_MILLS >=
                    light_state->toggle_interval) {
                    light_state->toggle_interval = utils_map(
                        abs_erpm, 300.00, 4000.00, LIGHT_BLINK_TIME_MAX, LIGHT_BLINK_TIME_MIN
                    );  // 映射abs_erpm 為閃爍時間
                    light_state->headlight_state = !light_state->headlight_state;
                    light_state->brakelight_state = !light_state->headlight_state;  // 交替状态
                    set_custom_headlight(light_state->headlight_state);
                    set_custom_brakelight(light_state->brakelight_state);
                    light_state->last_toggle_time = system_time;
                }

            } else if (light_state->light_mode == COB_LIGHT_FULL_ON) {
                set_custom_headlight(true);
                set_custom_brakelight(true);
            }

        } else {  // 如果煞車，依照設定的閃爍時間間隔閃爍煞車燈
        }
    } else {  // 非馬達轉動時間 , 如果燈模式為全亮則開始計時,閒置多久關閉燈並產生buzzer

        if (light_state->light_mode == COB_LIGHT_FULL_ON) {
            set_custom_headlight(true);
            set_custom_brakelight(true);

        } else {
            set_custom_headlight(false);
            set_custom_brakelight(false);
        }
    }
}

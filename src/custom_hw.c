#include "custom_hw.h"
#include <stdint.h>
#include <stdbool.h>

struct {
    bool brk_light_state, light_state, is_head_light_off, is_brake_light_off, brk_detect;
    float light_blink_time, brk_light_blink_time, light_blink_timer, idle_warning_time_out;
    uint8_t sound_triggered;
} custom_data;


float utils_map(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float get_idle_warning_timer(FLOAT_IDLE_TIME mode) {
    switch (mode) {
        case IDLE_WARNING_TIME_DISABLE: return 0;
        case IDLE_WARNING_TIME_1M: return 60;
        case IDLE_WARNING_TIME_5M: return 300;
        case IDLE_WARNING_TIME_10M: return 600;
        case IDLE_WARNING_TIME_30M: return 1800;
        case IDLE_WARNING_TIME_60M: return 3600;
        case IDLE_WARNING_TIME_120M: return 7200;
        default: return 0;
    }
}


void csutom_hw_init(custom_data *d)
{
	VESC_IF->set_pad_mode(FWD_LIGHT_GPIO, FWD_LIGHT_PIN, PAL_STM32_MODE_OUTPUT | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL);
	VESC_IF->set_pad_mode(REAR_LIGHT_GPIO, REAR_LIGHT_PIN, PAL_STM32_MODE_OUTPUT | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL);
	VESC_IF->set_pad_mode(EXT_DCDC_GPIO, EXT_DCDC_PIN, PAL_STM32_MODE_OUTPUT | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL);
}

/**
 * @brief 
 * 
 * @param d 
 * @param current_time 
 */
static void flash_the_lights(custom_data *d , float current_time ) {
    if ((current_time - d->light_blink_time) * SEC_TO_MILLS > d->light_blink_timer) {
        d->light_blink_timer = utils_map(d->abs_erpm, 0, LIGHT_BLINK_TIME_MAX, LIGHT_BLINK_TIME_MAX, LIGHT_BLINK_TIME_MIN);
        if (d->light_blink_timer < LIGHT_BLINK_TIME_MIN)
            d->light_blink_timer = LIGHT_BLINK_TIME_MIN;
        d->light_state = !d->light_state;
        d->light_blink_time = current_time;
    }
  d->light_state ? (FWD_LIGHT_ON(), REAR_LIGHT_OFF()) : (FWD_LIGHT_OFF(), REAR_LIGHT_ON());
}


static void check_lights(custom_data *d, const MotorData *motor ,State *state , cfgCustomHw *custom_config  ,float current_time  ,float pid_value )

{

	if (d->light_blink_time == 0)
		d->light_blink_time = current_time;
	if (d->brk_light_blink_time == 0)
		d->brk_light_blink_time = current_time;

	if (motor->abs_erpm > 20 && state->mode == STATE_RUNNING) // motor spin
	{
		// reset power alert time ,because motor is running .

        if (pid_value > -6) {

			switch (custom_config->lights_mode) {

			case COB_LIGHT_OFF:
				FWD_LIGHT_OFF();
				REAR_LIGHT_OFF();
				break;

			case COB_LIGHT_FLASH, :
				turn_head_light_off(d);
				turn_brake_light_off(d);

				break;

			case COB_LIGHT_FULL_ON:
				turn_head_light_on(d);
				turn_brake_light_on(d);

				break;
			}
        }
		else  // brake detected
    	 {

                    // if erpm >0 is forward Brake , erpm < 0 if reverse brake
                    if ((d->current_time - d->brk_light_blink_time) * SEC_TO_MILLS >=
                        BRK_LIGHT_BLINK_TIME_MIN) {
                        d->brk_light_state = !d->brk_light_state;
                        d->brk_light_blink_time = d->current_time;
                    }

                    d->brk_light_state ? (turn_brake_light_on(d)) : (turn_brake_light_off(d));
        }
    }
       //空閒時，除了設定燈光模式為全開外，其他設定全部關閉。
	if (d->state > RUNNING  || d->state == STARTUP )
 
	{
		if (d->float_conf.lights_mode == FLOAT_LIGHT_FULL_ON)
		{
			turn_head_light_on(d);
			turn_brake_light_on(d);
		}
		else
		{
			turn_head_light_off(d);
			turn_brake_light_off(d);
		}
	}
}

void reset_lights(data *d)
{
	// reset time var
	d->light_blink_time = 0;
	d->brk_light_blink_time = 0;
	d->idle_warning_time_out=get_idle_warning_timer(d->float_conf.idle_warning_time);
	// light blink timer
	d->light_blink_timer = LIGHT_BLINK_TIME_MAX;
	d->is_brake_light_off=true;
	d->is_head_light_off=true;
	// init the light mode
	if ((d->float_conf.lights_mode) == FLOAT_LIGHT_FULL_ON)
	{
	turn_brake_light_on(d);
	turn_head_light_on(d);
	}
	else
	{
	turn_brake_light_off(d);
	turn_head_light_off(d);
	}
	
}
// reset soune triggered
void reset_sound_triggered(data *d)

{
	d->sound_triggered = 0;
}

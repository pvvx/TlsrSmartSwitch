#include "app_main.h"

#if USE_SWITCH

/*******************************************************************
 * @brief	switch Action
 */
void switchAction(switch_action_t action, u8 count) {
//    bool report = false;
    zcl_msInputAttr_t *msInputAttr = zcl_msInputAttrsGet();

    sws_printf("switchAction(%d,%d)\n", action, count);

	if(count >= BUTTON_FR_COUNTER_MAX) {
		factory_reset_start(NULL);
		return;
	}

	if (cfg_on_off.switchType == ZCL_SWITCH_TYPE_MULTIFUNCTION) {
		if(action == SW_ACTION_END) {
			if(count <= ACTION_QUINTUPLE) {
				msInputAttr->value = count;
				app_forcedReport(APP_ENDPOINT1,
						ZCL_CLUSTER_GEN_MULTISTATE_INPUT_BASIC,
						ZCL_MULTISTATE_INPUT_ATTRID_PRESENT_VALUE);
			}
		}
	} else if(action < SW_ACTION_END) {
		// action == SW_ACTION_OFF or SW_ACTION_ON
		if (cfg_on_off.switchType == ZCL_SWITCH_TYPE_MOMENTARY) {
			if (cfg_on_off.switchActions == ZCL_SWITCH_ACTION_OFF_ON) {
				action = !action;
			}
			remoteCmdOnOff(action);
			if (cfg_on_off.switchDecoupled == CUSTOM_SWITCH_DECOUPLED_OFF) {
				cmdOnOff_set(action);
			}
		} else if (action == SW_ACTION_ON) {
			// cfg_on_off.switchType == ZCL_SWITCH_TYPE_TOGGLE
			remoteCmdOnOff(ZCL_CMD_ONOFF_TOGGLE);
			if (cfg_on_off.switchDecoupled == CUSTOM_SWITCH_DECOUPLED_OFF) {
				cmdOnOff_toggle();
			}
		}
	}
}

/*******************************************************************
 * @brief	switchFirstStart
 */
void switchFirstStart(void) {
	u8 OnOff = app_button[1].pressed;
	if (app_button[1].gpio_name
	&& cfg_on_off.switchType == ZCL_SWITCH_TYPE_MOMENTARY
	&& cfg_on_off.switchDecoupled == CUSTOM_SWITCH_DECOUPLED_OFF) {
		if (cfg_on_off.switchActions == ZCL_SWITCH_ACTION_OFF_ON) {
			OnOff = !OnOff;
		}
		cmdOnOff_set(OnOff);
	}
}


#endif // USE_SWITCH

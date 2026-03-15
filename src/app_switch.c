#include "app_main.h"

#if USE_SWITCH

static ev_timer_event_t *msInputTimerEvt = NULL;
static int32_t msInputTimerCb(void *args) {
	zcl_msInputAttr_t *msInputAttr = zcl_msInputAttrsGet();
	if(msInputAttr->value != ACTION_RELEASE) {
		msInputAttr->value = ACTION_RELEASE;
		wrk_rpt.extraSend = 1;
	}
	msInputTimerEvt = NULL;
    return -1;
}
/*******************************************************************
 * @brief	switch Action
 */
void switchAction(switch_action_t action, u8 count) {
//    bool report = false;
    zcl_msInputAttr_t *msInputAttr = zcl_msInputAttrsGet();

    sws_printf("switchAction(%d,%d)\n", action, count);

	if(count == BUTTON_CNT_FACTORY_RESET) {
		factory_reset_start(NULL);
		return;
	}
	if (cfg_on_off.switchType == ZCL_SWITCH_TYPE_MULTIFUNCTION) {
		if(action == SW_ACTION_END) {
			if(count != BUTTON_CNT_LONG_PRESS) {
			    if(!msInputTimerEvt) {
			    	msInputTimerEvt = TL_ZB_TIMER_SCHEDULE(msInputTimerCb, 0, TIMEOUT_500MS);
			    }
				msInputAttr->value = count;
				wrk_rpt.extraSend = 1;
			}
		} else if(count == BUTTON_CNT_LONG_PRESS) {
		    if(msInputTimerEvt) {
		    	TL_ZB_TIMER_CANCEL(&msInputTimerEvt);
		    	msInputTimerEvt = NULL;
		    }
			if(action == SW_ACTION_OFF)
				msInputAttr->value = ACTION_RELEASE;
			else // action == SW_ACTION_ON
				msInputAttr->value = ACTION_HOLD;
			wrk_rpt.extraSend = 1;
		} else if (action == SW_ACTION_ON && msInputAttr->value != ACTION_RELEASE) {
		    if(msInputTimerEvt) {
		    	TL_ZB_TIMER_CANCEL(&msInputTimerEvt);
		    	msInputTimerEvt = NULL;
		    }
			msInputAttr->value = ACTION_RELEASE;
			wrk_rpt.extraSend = 1;
		}
	} else if (cfg_on_off.switchType == ZCL_SWITCH_TYPE_MOMENTARY) {
		if(action < SW_ACTION_END) {
			action = ZCL_CMD_ONOFF_OFF;
		}
		if (cfg_on_off.switchActions == ZCL_SWITCH_ACTION_OFF_ON) {
			action = !action;
		}
		remoteCmdOnOff(action);
		if (cfg_on_off.switchDecoupled == CUSTOM_SWITCH_DECOUPLED_OFF) {
			cmdOnOff_set(action);
		}
	} else if (action == SW_ACTION_ON && count != BUTTON_CNT_LONG_PRESS) {
		// cfg_on_off.switchType == ZCL_SWITCH_TYPE_TOGGLE
		remoteCmdOnOff(ZCL_CMD_ONOFF_TOGGLE);
		if (cfg_on_off.switchDecoupled == CUSTOM_SWITCH_DECOUPLED_OFF) {
			cmdOnOff_toggle();
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

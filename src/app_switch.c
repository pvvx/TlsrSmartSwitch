#include "app_main.h"

#if USE_SWITCH

#define DEBOUNCE_SWITCH     128     /* number of polls for debounce */
#define FR_COUNTER_MAX      10      /* number for factory reset */

typedef enum {
	SWITCH_OFF = 0,
    SWITCH_ON,
    SWITCH_FLOAT,
} switch_status_t;

typedef struct {
    ev_timer_event_t *timerFrCounterEvt;
    ev_timer_event_t *timerMfCounterEvt;
    switch_status_t   status;
    bool              hold;
    uint8_t           debounce;
    uint8_t           counter;
//    uint8_t           fr_counter;             /* factory reset counter                        */
    uint32_t          time_hold;
} app_switch_t;

static app_switch_t app_switch = {
        .status = SWITCH_FLOAT,
        .debounce = (DEBOUNCE_SWITCH / 2),
        .hold = false,
        .counter = 0,
        .timerFrCounterEvt = NULL,
        .timerMfCounterEvt = NULL,
};

static int32_t switch_counterFrCb(void *args) {

    if (app_switch.counter >= FR_COUNTER_MAX) {
        TL_SCHEDULE_TASK(factory_reset_start, NULL);
    }

    if (app_switch.status == SWITCH_OFF
    && cfg_on_off.switchType != ZCL_SWITCH_TYPE_MULTIFUNCTION) {
        if (cfg_on_off.switchType == ZCL_SWITCH_TYPE_MOMENTARY) {
            if (cfg_on_off.switchActions == ZCL_SWITCH_ACTION_OFF_ON) {
                remoteCmdOnOff(ZCL_CMD_ONOFF_OFF);
                if (cfg_on_off.switchDecoupled == CUSTOM_SWITCH_DECOUPLED_OFF) {
                    cmdOnOff_off();
                }
            } else {
                remoteCmdOnOff(ZCL_CMD_ONOFF_ON);
                if (cfg_on_off.switchDecoupled == CUSTOM_SWITCH_DECOUPLED_OFF) {
                    cmdOnOff_on();
                }
            }
        }
    }

    app_switch.counter = 0;
    app_switch.timerFrCounterEvt = NULL;

    return -1;
}

static int32_t switch_counterMfCb(void *args) {

    //uint8_t i = (uint8_t)((uint32_t)args);
    bool report = false;

    zcl_msInputAttr_t *msInputAttr = zcl_msInputAttrsGet();

//    printf("counter: %d\r\n", app_switch[i].counter);

    if (app_switch.hold) {
        msInputAttr->value = ACTION_HOLD;                               // 0
        report = true;
    } else {
        switch(app_switch.counter) {
            case ACTION_SINGLE:                                         // 1
            case ACTION_DOUBLE:                                         // 2
            case ACTION_TRIPLE:                                         // 3
            case ACTION_QUADRUPLE:                                      // 4
            case ACTION_QUINTUPLE:                                      // 5
                msInputAttr->value = app_switch.counter;
                report = true;
                break;
            case ACTION_CLEAR:                                          // 250
                msInputAttr->value = ACTION_EMPTY;                      // 300
                report = true;
                break;
            default:
                if (app_switch.counter >= FR_COUNTER_MAX) {
                    TL_SCHEDULE_TASK(factory_reset_start, NULL);
                }
                break;
        }
    }

    if (report) {
        app_forcedReport(APP_ENDPOINT1, ZCL_CLUSTER_GEN_MULTISTATE_INPUT_BASIC, ZCL_MULTISTATE_INPUT_ATTRID_PRESENT_VALUE);
    }

    app_switch.counter = 0;
    app_switch.timerMfCounterEvt = NULL;

    return -1;
}

static void switch_first_start(switch_status_t status) {

    switch(cfg_on_off.startUpOnOff) {
        case ZCL_START_UP_ONOFF_SET_ONOFF_TO_PREVIOUS:
            if (cfg_on_off.onOff)
            	cmdOnOff_on();
            else
            	cmdOnOff_off();
            break;
        case ZCL_START_UP_ONOFF_SET_ONOFF_TOGGLE:
            cmdOnOff_toggle();
            break;
        case ZCL_START_UP_ONOFF_SET_ONOFF_TO_ON:
            cmdOnOff_on();
            break;
        case ZCL_START_UP_ONOFF_SET_ONOFF_TO_OFF:
            cmdOnOff_off();
            break;
        default:
            cmdOnOff_off();
            break;
    }
    app_switch.status = status;
}

static void read_switch_toggle(void) {
#if	GPIO_SWITCH_ON
    if (!gpio_read(dev_gpios.sw1))
#else
    if (gpio_read(dev_gpios.sw1))
#endif
    {
        if (app_switch.status != SWITCH_ON) {
            if (app_switch.debounce != DEBOUNCE_SWITCH) {
                app_switch.debounce++;
                if (app_switch.debounce == DEBOUNCE_SWITCH) {
                    if (app_switch.status == SWITCH_FLOAT) {
                        switch_first_start(SWITCH_ON);
                    } else {
                        app_switch.status = SWITCH_ON;
                        app_switch.counter++;
                        ev_timer_event_t *timerFrCounterEvt = app_switch.timerFrCounterEvt;
                        if (app_switch.timerFrCounterEvt) {
                            TL_ZB_TIMER_CANCEL(&timerFrCounterEvt);
                        }
                        app_switch.timerFrCounterEvt = TL_ZB_TIMER_SCHEDULE(switch_counterFrCb, NULL, TIMEOUT_1SEC);
                        if (app_switch.counter == 1) {
#if UART_PRINTF_MODE && DEBUG_SWITCH
                            printf("Switch is ON\r\n");
#endif
                            if (cfg_on_off.switchType == ZCL_SWITCH_TYPE_MOMENTARY) {
                                if (cfg_on_off.switchActions == ZCL_SWITCH_ACTION_OFF_ON) {
                                    remoteCmdOnOff(ZCL_CMD_ONOFF_ON);
                                    if (cfg_on_off.switchDecoupled == CUSTOM_SWITCH_DECOUPLED_OFF) {
                                        cmdOnOff_on();
                                    }
                                }
                                else {
                                    remoteCmdOnOff(ZCL_CMD_ONOFF_OFF);
                                    if (cfg_on_off.switchDecoupled == CUSTOM_SWITCH_DECOUPLED_OFF) {
                                        cmdOnOff_off();
                                    }
                                }
                            } else {
                                remoteCmdOnOff(ZCL_CMD_ONOFF_TOGGLE);
                                if (cfg_on_off.switchDecoupled == CUSTOM_SWITCH_DECOUPLED_OFF) {
                                    cmdOnOff_toggle();
                                }
                            }
                        }
                    }
                }
            }
        }
    } else {
        if (app_switch.status != SWITCH_OFF) {
            if (app_switch.debounce != 1) {
                app_switch.debounce--;
                if (app_switch.debounce == 1) {
                    if (app_switch.status == SWITCH_FLOAT) {
                        switch_first_start(SWITCH_OFF);
                    } else {
                        app_switch.status = SWITCH_OFF;
                        if (!app_switch.timerFrCounterEvt) {
#if UART_PRINTF_MODE && DEBUG_SWITCH
                            printf("Switch is OFF\r\n");
#endif
                            if (cfg_on_off.switchType == ZCL_SWITCH_TYPE_MOMENTARY) {
                                if (cfg_on_off.switchActions == ZCL_SWITCH_ACTION_OFF_ON) {
                                    remoteCmdOnOff(ZCL_CMD_ONOFF_OFF);
                                    if (cfg_on_off.switchDecoupled == CUSTOM_SWITCH_DECOUPLED_OFF) {
                                        cmdOnOff_off();
                                    }
                                } else {
                                    remoteCmdOnOff(ZCL_CMD_ONOFF_ON);
                                    if (cfg_on_off.switchDecoupled == CUSTOM_SWITCH_DECOUPLED_OFF) {
                                        cmdOnOff_on();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

static void read_switch_multifunction(void) {

    zcl_msInputAttr_t *msInputAttr = zcl_msInputAttrsGet();
#if	GPIO_SWITCH_ON
    if (!gpio_read(dev_gpios.sw1))
#else
    if (gpio_read(dev_gpios.sw1))
#endif
    {
        if (app_switch.status == SWITCH_ON) {
            if (clock_time_exceed(app_switch.time_hold, TIMEOUT_TICK_500MS)) {
                if (!app_switch.hold) {
                    app_switch.hold = true;
//                    printf("on. hold: %d\r\n", app_switch.time_hold);
                }
            }
        } else {
            if (app_switch.debounce != DEBOUNCE_SWITCH) {
                app_switch.debounce++;
                if (app_switch.debounce == DEBOUNCE_SWITCH) {
                    if (app_switch.status == SWITCH_FLOAT) {
                        app_switch.counter = ACTION_CLEAR;
                        app_switch.status = SWITCH_ON;
                        switch_counterMfCb(NULL);
                    } else {
                        app_switch.status = SWITCH_ON;
                        app_switch.counter++;
                        ev_timer_event_t *timerMfCounterEvt = app_switch.timerMfCounterEvt;
                        if (app_switch.timerMfCounterEvt) {
                            TL_ZB_TIMER_CANCEL(&timerMfCounterEvt);
                        }
                        app_switch.timerMfCounterEvt = TL_ZB_TIMER_SCHEDULE(switch_counterMfCb, NULL, TIMEOUT_500MS);
                    }
                }
            }
        }
    } else {
        if (app_switch.status == SWITCH_OFF) {
            app_switch.time_hold = clock_time();
            if (app_switch.hold) {
                app_switch.hold = false;
                msInputAttr->value = ACTION_RELEASE;
                app_forcedReport(APP_ENDPOINT1, ZCL_CLUSTER_GEN_MULTISTATE_INPUT_BASIC, ZCL_MULTISTATE_INPUT_ATTRID_PRESENT_VALUE);
            }
        } else {
            if (app_switch.debounce != 1) {
                app_switch.debounce--;
                if (app_switch.debounce == 1) {
                    if (app_switch.status == SWITCH_FLOAT) {
                        app_switch.counter = ACTION_CLEAR;
                        app_switch.status = SWITCH_OFF;
                        switch_counterMfCb(NULL);
                    } else {
                        app_switch.status = SWITCH_OFF;
                    }
                }
            }
        }
    }
}

void switch_handler(void) {
#if USE_SENSOR_MY18B20
	if(dev_gpios.sw1 == dev_gpios.swire
#if USE_THERMOSTAT
		&& cfg_on_off.switchType == ZCL_SWITCH_TYPE_TERMOSTAT
#endif
	) {
		return;
	}
#endif
	if (cfg_on_off.switchType == ZCL_SWITCH_TYPE_MULTIFUNCTION) {
       	read_switch_multifunction();
    } else {
        read_switch_toggle();
    }
}

bool switch_idle(void) {
#if USE_SENSOR_MY18B20
	if(dev_gpios.sw1 == dev_gpios.swire
#if USE_THERMOSTAT
		&& cfg_on_off.switchType == ZCL_SWITCH_TYPE_TERMOSTAT
#endif
	) {
		return false;
	}
#endif
    if (app_switch.timerFrCounterEvt)
    	return true;
    if (app_switch.debounce != 1 && app_switch.debounce != DEBOUNCE_SWITCH)
    	return true;
    return false;
}

#endif // USE_SWITCH

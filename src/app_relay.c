#include "app_main.h"
#include "sensors.h"


relay_settings_t relay_settings;
relay_settings_t relay_settings_saved; // use compare before nv_flashWriteNew()

const relay_settings_t relay_settings_default = {
    .status_onoff = {ZCL_ONOFF_STATUS_OFF},
    .startUpOnOff = {ZCL_START_UP_ONOFF_SET_ONOFF_TO_OFF},
#if USE_SWITCH
    .switchType = {ZCL_SWITCH_TYPE_MOMENTARY},
    .switchActions = {ZCL_SWITCH_ACTION_OFF_ON},
    .switch_decoupled = {CUSTOM_SWITCH_DECOUPLED_OFF}
#endif
};

dev_relay_t      dev_relay;

#if USE_SWITCH
static void check_first_start(uint8_t i) {

    switch(relay_settings.startUpOnOff[i]) {
        case ZCL_START_UP_ONOFF_SET_ONOFF_TO_PREVIOUS:
            if (relay_settings.status_onoff[i]) cmdOnOff_on(dev_relay.unit_relay[i].ep);
            else cmdOnOff_off(dev_relay.unit_relay[i].ep);
            break;
        case ZCL_START_UP_ONOFF_SET_ONOFF_TOGGLE:
            cmdOnOff_toggle(dev_relay.unit_relay[i].ep);
            break;
        case ZCL_START_UP_ONOFF_SET_ONOFF_TO_ON:
            cmdOnOff_on(dev_relay.unit_relay[i].ep);
            break;
        case ZCL_START_UP_ONOFF_SET_ONOFF_TO_OFF:
            cmdOnOff_off(dev_relay.unit_relay[i].ep);
            break;
        default:
            cmdOnOff_off(dev_relay.unit_relay[i].ep);
            break;
    }
}

#endif

void set_relay_status(uint8_t i, uint8_t status) {
//    printf("set_relay_status(i = %d, status = %d). GPIO: %d\r\n", i, status, dev_relay.unit_relay[i].rl);
	if(status) {
		if(tik_reload != 0xffff || tik_start != 0xffff)
			status = 0;
	}
	drv_gpio_write(dev_relay.unit_relay[i].rl, status);
}

nv_sts_t relay_settings_save(void) {
    nv_sts_t st = NV_SUCC;
#if NV_ENABLE
 #if UART_PRINTF_MODE
    printf("Saved relay settings\r\n");
 #endif
    if(memcmp(&relay_settings, &relay_settings_saved, sizeof(relay_settings))) {
    	st = nv_flashWriteNew(1, NV_MODULE_APP,  NV_ITEM_APP_CFG_RELAY, sizeof(relay_settings_t), (uint8_t*)&relay_settings);
    	memcpy(&relay_settings_saved, &relay_settings, (sizeof(relay_settings_saved)));
    }
#else
    st = NV_ENABLE_PROTECT_ERROR;
#endif
    return st;
}

static void relay_settints_set(relay_settings_t * prls) {
    memcpy(&relay_settings_saved, prls, (sizeof(relay_settings_saved)));
    g_zcl_onOffAttrs[0].onOff = prls->status_onoff[0];
    g_zcl_onOffAttrs[0].startUpOnOff = prls->startUpOnOff[0];
#if USE_SWITCH
    g_zcl_onOffCfgAttrs[0].custom_swtichType = g_zcl_onOffCfgAttrs[0].switchType = prls->switchType[0];
    g_zcl_onOffCfgAttrs[0].switchActions = prls->switchActions[0];
    g_zcl_onOffCfgAttrs[0].custom_decoupled = prls->switch_decoupled[0];
#endif
}

void relay_settints_default(void) {
	memcpy(&relay_settings, &relay_settings_default, (sizeof(relay_settings)));
    relay_settings_save();
	relay_settints_set(&relay_settings);
}

nv_sts_t relay_settings_restore(void) {
    nv_sts_t st = NV_SUCC;

#if NV_ENABLE

    st = nv_flashReadNew(1, NV_MODULE_APP,  NV_ITEM_APP_CFG_RELAY, sizeof(relay_settings_t), (uint8_t*)&relay_settings);

    if (st == NV_SUCC) {

#if UART_PRINTF_MODE
        printf("Restored relay settings\r\n");
#if DEBUG_SAVE
        print_setting_sr(st, &relay_settings_tmp, false);
#endif
#endif

    } else {
        /* default config */
        memcpy(&relay_settings, &relay_settings_default, (sizeof(relay_settings_t)));
#if UART_PRINTF_MODE
        printf("Default relay settings \r\n");
#endif
    }

    relay_settints_set(&relay_settings);

#else
    st = NV_ENABLE_PROTECT_ERROR;
#endif

    return st;
}

void dev_relay_init(void) {
    dev_relay.amt = AMT_RELAY;
    dev_relay.unit_relay[0].ep = APP_ENDPOINT1;
#if USE_SWITCH
    dev_relay.unit_relay[0].sw = SWITCH1_GPIO;
#endif
    dev_relay.unit_relay[0].rl = RELAY1_GPIO;
#if USE_SWITCH
    if (relay_settings.switchType[0] == ZCL_SWITCH_TYPE_MULTIFUNCTION) {
        check_first_start(ZCL_START_UP_ONOFF_SET_ONOFF_TO_OFF);
    }
#endif
}



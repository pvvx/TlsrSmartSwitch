#include "app_main.h"
#if USE_SENSOR_MY18B20
#include "my18b20.h"
#endif


void cmdOnOff_set(bool status) {
	sws_printf("cmdOnOff_set(%d)\n", status);
    if(cfg_on_off_saved.onOff != status) {
    	cfg_on_off.onOff = status;
    	if(cfg_on_off.startUpOnOff != ZCL_START_UP_ONOFF_SET_ONOFF_TO_OFF
   			&& cfg_on_off.startUpOnOff != ZCL_START_UP_ONOFF_SET_ONOFF_TO_ON) {
        	save_config_on_off();
    	} else {
    		cfg_on_off_saved.onOff = cfg_on_off.onOff;
    	}
    }
#if USE_THERMOSTAT // USE_SENSOR_MY18B20
    set_therm_relay_status(status);
#else
	set_relay_status(status);
#endif
}

void cmdOnOff_toggle(void) {
    cmdOnOff_set(!cfg_on_off_saved.onOff);
}

void remoteCmdOnOff(uint8_t cmd) {
	if (zb_isDeviceJoinedNwk()) {

		epInfo_t dstEpInfo;
		TL_SETSTRUCTCONTENT(dstEpInfo, 0);

		dstEpInfo.profileId = HA_PROFILE_ID;

		dstEpInfo.dstAddrMode = APS_DSTADDR_EP_NOTPRESETNT;

		/* command 0x00 - off, 0x01 - on, 0x02 - toggle */

		sws_printf("remoteCmdOnOff(%d)\n", cmd);

		switch (cmd) {
		case ZCL_CMD_ONOFF_OFF:
			zcl_onOff_offCmd(APP_ENDPOINT1, &dstEpInfo, FALSE);
			break;
		case ZCL_CMD_ONOFF_ON:
			zcl_onOff_onCmd(APP_ENDPOINT1, &dstEpInfo, FALSE);
			break;
		case ZCL_CMD_ONOFF_TOGGLE:
			zcl_onOff_toggleCmd(APP_ENDPOINT1, &dstEpInfo, FALSE);
			break;
		default:
			break;
		}
	}
}

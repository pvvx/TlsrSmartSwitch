#ifndef _INCLUDE_APP_ONOFF_H_
#define _INCLUDE_APP_ONOFF_H_

void cmdOnOff_set(bool status);

inline void cmdOnOff_on(void) {
	cmdOnOff_set(ZCL_ONOFF_STATUS_ON);
}
inline void cmdOnOff_off(void){
	cmdOnOff_set(ZCL_ONOFF_STATUS_OFF);
}
void cmdOnOff_toggle(void);

void remoteCmdOnOff(uint8_t cmd);

#endif /* _INCLUDE_APP_ONOFF_H_ */

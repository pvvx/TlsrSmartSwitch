#ifndef SRC_INCLUDE_APP_REPORTING_H_
#define SRC_INCLUDE_APP_REPORTING_H_

typedef struct {
	u32 oldTimeSec;
	u32 oldSysTick;
	u8  cntRepeat;
	u8  extraSend;
} app_wrk_report_t;

extern app_wrk_report_t wrk_rpt;

status_t app_forcedReport(uint8_t endpoint, uint16_t claster_id, uint16_t attr_id);

//void reportAttrTimerStop(void);
//void reportAttrTimerStart(void);
status_t app_chk_report(u16 uptime_sec);
void app_report_handler(void);

#endif /* SRC_INCLUDE_APP_REPORTING_H_ */

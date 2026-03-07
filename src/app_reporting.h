#ifndef SRC_INCLUDE_APP_REPORTING_H_
#define SRC_INCLUDE_APP_REPORTING_H_

extern uint32_t last_timeReportMsi;
extern uint8_t  last_seqNum;

status_t app_forcedReport(uint8_t endpoint, uint16_t claster_id, uint16_t attr_id);

//void reportAttrTimerStop(void);
//void reportAttrTimerStart(void);
status_t app_chk_report(u16 uptime_sec);
void app_set_report(u16 clusterID);
void app_report_handler(void);

#endif /* SRC_INCLUDE_APP_REPORTING_H_ */

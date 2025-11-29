/*
 * battery.h
 *
 *  Created on: 24 янв. 2025 г.
 *      Author: pvvx
 */

#ifndef _BATTERY_H_
#define _BATTERY_H_

#define BATTERY_MAX_POWER			3400//3.4v
#define BATTERY_LOW_POWER			2500//2.5v
#define BATTERY_START_POWER			2700//2.7v
#define LOW_POWER_SLEEP_TIME_ms		30*1000 // 30 sec

#if ZCL_POWER_CFG_SUPPORT
// measured_battery.flag:
#define FLG_MEASURE_BAT_ADV		0x01
#define FLG_MEASURE_BAT_CC		0x02

typedef struct _measured_battery_t {
	u32 summ;		// сумматор
	u16	mv; 		// mV
	u16	average_mv; // mV
	u16 cnt;
	u8	level; 		// in 0.5% 0..200
#if USE_BLE
	u8  batVal; 	// 0..100%
#endif
	u8  flag;
} measured_battery_t;

extern measured_battery_t measured_battery;

#endif

void adc_channel_init(ADC_InputPchTypeDef p_ain); // in adc_drv.c
u16 get_adc_mv(int flg); // in adc_drv.c

void battery_detect(bool startup_flg);

#endif /* _BATTERY_H_ */

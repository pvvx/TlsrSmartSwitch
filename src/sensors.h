/*
 * sensor.h
 *
 *  Created on: 14 нояб. 2023 г.
 *      Author: pvvx
 */

#ifndef _SENSORS_H_
#define _SENSORS_H_

enum {
	TH_SENSOR_NONE = 0,
	TH_SENSOR_SHTC3,   // 1
	TH_SENSOR_SHT4x,   // 2
	TH_SENSOR_SHT30,	// 3 CHT8320
	TH_SENSOR_CHT8305,	// 4
	TH_SENSOR_AHT2x,	// 5
	TH_SENSOR_CHT8215,	// 6
	IU_SENSOR_INA226,	// 7
	IU_SENSOR_MY18B20,	// 8
	IU_SENSOR_MY18B20x2,	// 9
	IU_SENSOR_HX71X,	// 10
	IU_SENSOR_PWMRH,	// 11
	IU_SENSOR_NTC,		// 12
	IU_SENSOR_BL0937,	// 13
	IU_SENSOR_BL0942,	// 14
	TH_SENSOR_TYPE_MAX // 15
} SENSOR_TYPES_e;

uint16_t tik_reload, tik_start; // step 1 sec, =0xFFFF - flag end

#endif /* _SENSORS_H_ */

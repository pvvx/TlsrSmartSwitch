/*
 * my18b20.h
 *
 *  Created on: 3 авг. 2024 г.
 *      Author: pvvx
 */

#ifndef _MY18B20_H_
#define _MY18B20_H_

typedef struct _my18b20_coef_t {
	u32 temp_k;
	s16 temp_z;
	s16 temp_hysteresis; // in 0.1 С
} my18b20_coef_t;

// extern my18b20_coef_t def_coef_my18b20;

typedef struct {
	my18b20_coef_t coef;
	s16 rtemp;
	u32 id;
	u8	start_measure;
	u8	stage;
	u8	type;
	u8	temp_trg_on;
	u32 tick;
	u32 timeout;
} my18b20_t;

extern my18b20_t my18b20;

nv_sts_t load_config_my18b20(void);
nv_sts_t save_config_my18b20(void);

nv_sts_t load_config_termostat(void);
nv_sts_t save_config_termostat(void);

void init_my18b20(void);
void task_my18b20(void);

#endif /* _MY18B20_H_ */

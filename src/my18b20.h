/*
 * my18b20.h
 *
 *  Created on: 3 авг. 2024 г.
 *      Author: pvvx
 */

#ifndef _MY18B20_H_
#define _MY18B20_H_

typedef struct _my18b20_coef_t {
	uint32_t temp_k;
	int16_t temp_z; // in 0.01 C
	int16_t temp_hysteresis; // in 0.01 С
	int16_t min_temp; // in 0.01 C
	int16_t max_temp; // in 0.01 C
} my18b20_coef_t;

extern zcl_thermostatAttr_save_t thr_cfg_saved;

// extern my18b20_coef_t def_coef_my18b20;

typedef enum {
	MY18B20_BIT_ERR_INIT = 0,
	MY18B20_BIT_ERR_READ,
	MY18B20_BIT_ERR_BAD
} my18b20_errors_e;

typedef struct {
	my18b20_coef_t coef;
	int16_t rtemp;
	uint32_t id;
	uint8_t	start_measure;
	uint8_t	stage;
	uint8_t	type;
	uint8_t	temp_trg_on;
	uint32_t tick;
	uint32_t timeout;
	uint8_t errors; // AC_ERROR_CODE?
	uint8_t cnt_errors;
} my18b20_t;

extern my18b20_t my18b20;

nv_sts_t load_config_my18b20(void);
nv_sts_t save_config_my18b20(void);

nv_sts_t load_config_termostat(void);
nv_sts_t save_config_termostat(void);

void init_my18b20(void);
void task_my18b20(void);

#endif /* _MY18B20_H_ */

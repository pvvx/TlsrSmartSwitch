/*
 * my18b20.c
 *
 *  Created on: 2 авг. 2024 г.
 *      Author: pvvx
 */
#include "app_main.h"
#if USE_SENSOR_MY18B20
#include "sensors.h"
#include "my18b20.h"

// #define GPIO_ONEWIRE	GPIO_PB1

#define ERROR_TIMEOUT_TICK		(200 * CLOCK_16M_SYS_TIMER_CLK_1MS)
#define MEASURE_TIMEOUT_TICK	(500 * CLOCK_16M_SYS_TIMER_CLK_1MS)
#define MIN_STEP_TICK_US		495
#define MIN_STEP_TICK			(MIN_STEP_TICK_US * CLOCK_16M_SYS_TIMER_CLK_1US)
#define POST_PRESENT_STEP_TICK		((480-60) * CLOCK_16M_SYS_TIMER_CLK_1US)

const my18b20_coef_t coef_my18b20_def = {
		.temp_k = 409600, // temp_k
		.temp_z = 0, // temp_z in 0.01C
		.temp_hysteresis = 15, // in 0.01C
		.min_temp = -5000, // in 0.01 C
		.max_temp = 12500, // in 0.01 C
};

my18b20_coef_t coef_my18b20_saved;
my18b20_t my18b20;

static inline void onewire_pin_lo(void) {
	gpio_set_output_en(dev_gpios.swire, 1);
}

static inline void onewire_pin_hi(void) {
	gpio_set_output_en(dev_gpios.swire, 0);
}

static inline unsigned int onewire_pin_read(void) {
	return gpio_read(dev_gpios.swire);
}

static void onewire_bus_low(void) {
	gpio_setup_up_down_resistor(dev_gpios.swire, PM_PIN_PULLDOWN_100K);
	gpio_set_output_en(dev_gpios.swire, 1); // enable output
}

static void onewire_bus_hi(void) {
	gpio_setup_up_down_resistor(dev_gpios.swire, PM_PIN_PULLUP_10K);
	gpio_set_output_en(dev_gpios.swire, 0); // disable output
}


// return: -1 - error, 0..3 - ok
_attribute_ram_code_
static int onewire_bit_read(void) {
	int ret = 0;
	unsigned char r = irq_disable();
	onewire_pin_hi();
	sleep_us(15); // 15
	if(onewire_pin_read()) {
		unsigned int tt = clock_time();
		onewire_pin_lo();
		sleep_us(1); // 1..3
		onewire_pin_hi();
		sleep_us(15); // 15
		ret = onewire_pin_read();
		if(!ret) {
			while(!onewire_pin_read()) {
				if(clock_time() - tt > 65 * CLOCK_16M_SYS_TIMER_CLK_1US) {
					ret = -1;
					break;
				}
			}
		}
	} else
		ret = -1;
	irq_restore(r);
	return ret;
}

// return: -1 - error, 0 - ok
_attribute_ram_code_
static int onewire_bit_write(unsigned int cbit) {
	int ret = 0;
	unsigned char r = irq_disable();
	onewire_pin_hi();
	sleep_us(15);	// 15
	if(onewire_pin_read()) {
		onewire_pin_lo();
		sleep_us(1); // 1..3
		if(cbit)
			onewire_pin_hi();
		sleep_us(45); // 30
		onewire_pin_hi();
	} else
		ret = -1;
	irq_restore(r);
	return ret;
}

// return: -1 - error, 0 - ok
static int onewire_tst_presence(void) {
	int ret = -1;
	unsigned int tt;
	unsigned char r = irq_disable();
	onewire_bus_hi();
	sleep_us(80);	// 65..80
	if(!onewire_pin_read()) {
		irq_restore(r);
		ret = 0;
		tt = clock_time();
		while(!onewire_pin_read()) {
			if(clock_time() - tt > (265-80) * CLOCK_16M_SYS_TIMER_CLK_1US) {
				ret = -1;
				break;
			}
		}
	} else
		irq_restore(r);
	//while(clock_time() - tt < ((480-80) * CLOCK_16M_SYS_TIMER_CLK_1US));
	return ret;
}


// return: -1 - error, 0 - ok
static int onewire_write(unsigned int value, int bitcnt) {
	int ret = 0;
	while(bitcnt--) {
		ret = onewire_bit_write(value & 1);
		value >>= 1;
		if(ret < 0) {
			break;
		}
	}
	return ret;
}

// return: -1 - error, 0 - ok
static int onewire_16bit_read(short *pdata) {
	int ret = 0;
	unsigned int mask = 1;
	while(mask < 0x10000) {
		int cbit = onewire_bit_read();
		if(cbit < 0) {
			ret = -1;
			break;
		} else {
			if(cbit)
				pdata[0] |= mask;
			else
				pdata[0] &= ~mask;
		}
		mask <<= 1;
	}
	return ret;
}

static int onewire_read(unsigned int bitcnt) {
	int ret = 0;
	unsigned int mask = 1;
	while(bitcnt--) {
		int cbit = onewire_bit_read();
		if(cbit < 0) {
			ret = -1;
			break;
		} else
			if(cbit)
				ret |= mask;
		mask <<= 1;
	}
	return ret;
}

#ifdef ZCL_THERMOSTAT

#define SHL_SUMM_TEMP  2 // Bit

enum{
	TH_RMODE_OFF = 0,
	TH_RMODE_COOL = 3,
	TH_RMODE_HEAT = 4
};

enum{
	TH_WRK_BIT_HEAT = 0,
	TH_WRK_BIT_COOL = 1
};

static const uint8_t bit_th_sys_mode_tab[] = {
	0,  // TH_SMODE_OFF
	BIT(TH_WRK_BIT_HEAT) | BIT(TH_WRK_BIT_COOL),	// TH_SMODE_AUTO
	0, // none
	BIT(TH_WRK_BIT_COOL),	// TH_SMODE_COOL
	BIT(TH_WRK_BIT_HEAT),	// TH_SMODE_HEAT
	BIT(TH_WRK_BIT_HEAT),	// TH_SMODE_EHEAT
	BIT(TH_WRK_BIT_COOL)	// TH_SMODE_PRECOOL
};

static void set_thermostat(int16_t temp) {

	uint8_t bit_mode = 0;
	temp += (int16_t)zcl_thermostat_attrs.cfg.temp_z8;
	zcl_thermostat_attrs.local_temp = temp;

	if(zcl_thermostat_attrs.cfg.sys_mode >= sizeof(bit_th_sys_mode_tab))
		zcl_thermostat_attrs.cfg.sys_mode = TH_SMODE_OFF;
	bit_mode = bit_th_sys_mode_tab[zcl_thermostat_attrs.cfg.sys_mode];

	if(bit_mode == 0) {

		zcl_thermostat_attrs.cool_on = 0;
		zcl_thermostat_attrs.healt_on = 0;
		zcl_thermostat_attrs.relay_state = 0;

		zcl_thermostat_attrs.cfg.sys_mode = TH_SMODE_OFF;
		zcl_thermostat_attrs.run_mode = TH_RMODE_OFF;

	} else {

		if(bit_mode & BIT(TH_WRK_BIT_HEAT)) { // heat
			if(zcl_thermostat_attrs.healt_on) {
				if(temp > zcl_thermostat_attrs.cfg.temp_heating + my18b20.coef.temp_hysteresis) {
					zcl_thermostat_attrs.healt_on = 0;
					zcl_thermostat_attrs.relay_state = BIT(2);
				} else {
					// zcl_thermostat_attrs.healt_on = 100;
				}
			} else {
				if(temp < zcl_thermostat_attrs.cfg.temp_heating - my18b20.coef.temp_hysteresis) {
					zcl_thermostat_attrs.healt_on = 100;
					zcl_thermostat_attrs.relay_state = BIT(0) | BIT(2);
					// TODO: Alarm
				} else {
					// zcl_thermostat_attrs.healt_on = 0;
				}
			}
			if((bit_mode & BIT(TH_WRK_BIT_COOL)) == 0) {
				zcl_thermostat_attrs.cool_on = 0;
			}
		}
		if(bit_mode & BIT(TH_WRK_BIT_COOL)) { // cool
			if(zcl_thermostat_attrs.cool_on) {
				if(temp < zcl_thermostat_attrs.cfg.temp_cooling - my18b20.coef.temp_hysteresis) {
					zcl_thermostat_attrs.cool_on = 0;
					zcl_thermostat_attrs.relay_state = BIT(2);
				} else {
					// zcl_thermostat_attrs.healt_on = 100;
				}
			} else {
				if(temp > zcl_thermostat_attrs.cfg.temp_cooling + my18b20.coef.temp_hysteresis) {
					zcl_thermostat_attrs.cool_on = 100;
					zcl_thermostat_attrs.relay_state = BIT(1) | BIT(2);
					// TODO: Alarm
				} else {
					// zcl_thermostat_attrs.healt_on = 0;
				}
			}
			if((bit_mode & BIT(TH_WRK_BIT_COOL)) == 0)
				zcl_thermostat_attrs.healt_on = 0;
		}
		if(zcl_thermostat_attrs.healt_on) {
			zcl_thermostat_attrs.run_mode = TH_RMODE_HEAT;
		} else if(zcl_thermostat_attrs.cool_on) {
			zcl_thermostat_attrs.run_mode = TH_RMODE_COOL;
		} else {
			zcl_thermostat_attrs.run_mode = TH_RMODE_OFF;
		}
	}
	if(zcl_thermostat_attrs.cfg.sys_mode != TH_SMODE_OFF)
		set_relay_status(zcl_thermostat_attrs.run_mode != TH_RMODE_OFF);
}
#endif // ZCL_THERMOSTAT

void init_my18b20(void) {
	load_config_my18b20();
#ifdef ZCL_THERMOSTAT
	load_config_termostat();
#endif
	if(!dev_gpios.swire) {
		dev_gpios.swire = GPIO_ONEWIRE1;
	}
//	gpio_set_func(dev_gpios.swire, AS_GPIO);
	gpio_input_init(dev_gpios.swire, PM_PIN_PULLUP_10K);
	onewire_bus_low();
	my18b20.type = IU_SENSOR_MY18B20;
	my18b20.stage = 0; // init
	my18b20.timeout = MIN_STEP_TICK;
	sleep_us(MIN_STEP_TICK_US);
	if(onewire_tst_presence() >= 0) {
		sleep_us(POST_PRESENT_STEP_TICK);
		if(onewire_write(0x033, 8) >= 0) {
			my18b20.id = onewire_read(32);
		} else {
			my18b20.errors |= BIT(0); // Room Temperature Sensor Failure?
		}
	} else {
		my18b20.errors |= BIT(0); // Room Temperature Sensor Failure?
	}
	onewire_bus_low();
	my18b20.tick = clock_time();
}

static void error_my18b20(void) {
	my18b20.errors |= BIT(1);
	my18b20.stage = 0;
	if(++my18b20.cnt_errors > 7) {
		my18b20.errors |= BIT(2);
		my18b20.cnt_errors = 0;
#ifdef ZCL_TEMPERATURE_MEASUREMENT
		g_zcl_temperatureAttrs.measuredValue = 0x8000;
#endif
#ifdef ZCL_THERMOSTAT
		zcl_thermostat_attrs.local_temp = 0x8000;
#endif
#if USE_METERING
		if (config_min_max.emergency_off & BIT(BIT_ERR_TS_OFF)) {
			relay_off |= BIT(BIT_ERR_TS_OFF);
		}
#endif
	}
}

__attribute__((optimize("-O2"))) // No Os!
void task_my18b20(void) {
	int16_t temp;
	if(clock_time() - my18b20.tick > my18b20.timeout) {
		switch(my18b20.stage) {
		case 1: // init config reg
			if(onewire_write(0x0cc, 8) >= 0 // no addr
			  && onewire_write(0x7f00ff4e, 32) >= 0) { // init config reg
				my18b20.stage = 2;
				my18b20.timeout = 10;
			} else {
				error_my18b20();
				my18b20.timeout = ERROR_TIMEOUT_TICK;
			}
			onewire_bus_low();
			break;
		case 2:
			if(my18b20.start_measure) {
				my18b20.start_measure = 0;
				my18b20.stage = 3; // reset bus -> presence -> stage 4 (start measure)
				my18b20.timeout = 10;
			}
			break;
		case 4:	// start measure
			if(onewire_write(0x044cc, 16) >= 0) { // start measure
				my18b20.stage = 5;
				my18b20.timeout = MEASURE_TIMEOUT_TICK;
			} else {
				error_my18b20();
				my18b20.timeout = MIN_STEP_TICK;
				onewire_bus_low();
			}
			break;
		case 5:
			onewire_bus_low();
			my18b20.timeout = MIN_STEP_TICK;
			my18b20.stage = 6; // reset bus -> presence -> stage 7 (read measure)
			break;
		case 7: // read measure
			if(onewire_write(0x0becc, 16) >= 0 // cmd read
				&& onewire_16bit_read(&my18b20.rtemp) >= 0) { // read measure
				temp = ((int)(my18b20.rtemp * my18b20.coef.temp_k) >> 16) + my18b20.coef.temp_z; // x 0.01 C
#if USE_METERING
				if ((config_min_max.emergency_off & BIT(BIT_MAX_TEMP_OFF))
					&& temp > my18b20.coef.max_temp)
					relay_off |= BIT(BIT_MAX_TEMP_OFF);
				if ((config_min_max.emergency_off & BIT(BIT_MIN_TEMP_OFF))
					&& temp > my18b20.coef.min_temp)
					relay_off |= BIT(BIT_MIN_TEMP_OFF);
#endif
#ifdef ZCL_THERMOSTAT
				set_thermostat(temp);
#endif
#ifdef ZCL_TEMPERATURE_MEASUREMENT
				g_zcl_temperatureAttrs.measuredValue = temp;
#endif
				my18b20.stage = 2;
				my18b20.cnt_errors = 0;
			} else {
				error_my18b20();
			}
			onewire_bus_low();
			my18b20.timeout = MIN_STEP_TICK;
			break;

		default: // reset bus -> presence
			if(onewire_tst_presence() >= 0) {
				// presence ok
				my18b20.stage++;
				my18b20.timeout = POST_PRESENT_STEP_TICK;
			} else {
				// presence err
				error_my18b20();
				my18b20.timeout = MIN_STEP_TICK + POST_PRESENT_STEP_TICK;
				onewire_bus_low();
			}
		}
		my18b20.tick = clock_time();
	}
}

//--------- Loading/Saving Sensor Coefficients --------

nv_sts_t load_config_my18b20(void) {
#if NV_ENABLE
	nv_sts_t ret = nv_flashReadNew(1, NV_MODULE_APP,  NV_ITEM_APP_CFG_SENSOR_MY18B20, sizeof(coef_my18b20_saved), (uint8_t*)&coef_my18b20_saved);
	if(ret !=  NV_SUCC) {
		memcpy(&coef_my18b20_saved, &coef_my18b20_def, sizeof(coef_my18b20_saved));
	}
	memcpy(&my18b20.coef, &coef_my18b20_saved, sizeof(my18b20.coef));
	return ret;
#else
	memcpy(&my18b20.coef, &coef_my18b20_def, sizeof(my18b20.coef));
    return NV_ENABLE_PROTECT_ERROR;
#endif
}

nv_sts_t save_config_my18b20(void) {
#if NV_ENABLE
	nv_sts_t ret = NV_SUCC;
	if(memcmp(&coef_my18b20_saved, &my18b20.coef, sizeof(coef_my18b20_saved))) {
		memcpy(&coef_my18b20_saved, &my18b20.coef, sizeof(coef_my18b20_saved));
		ret = nv_flashWriteNew(1, NV_MODULE_APP,  NV_ITEM_APP_CFG_SENSOR_MY18B20, sizeof(coef_my18b20_saved), (uint8_t*)&coef_my18b20_saved);
	}
    return ret;
#else
    return NV_ENABLE_PROTECT_ERROR;
#endif
}

#ifdef ZCL_THERMOSTAT

zcl_thermostatAttr_save_t thr_cfg_saved;
const zcl_thermostatAttr_save_t thr_cfg_def = {
	.temp_cooling = 2400,
	.temp_heating = 2100,
	.sys_mode = TH_SMODE_OFF,
	.temp_z8 = 0
};

nv_sts_t load_config_termostat(void) {
#if NV_ENABLE
	nv_sts_t ret = nv_flashReadNew(1, NV_MODULE_APP,  NV_ITEM_APP_CFG_THERMOSTAT, sizeof(thr_cfg_saved), (uint8_t*)&thr_cfg_saved);
	if(ret !=  NV_SUCC) {
		memcpy(&thr_cfg_saved, &thr_cfg_def, sizeof(thr_cfg_saved));
	}
	memcpy(&zcl_thermostat_attrs.cfg, &thr_cfg_saved, sizeof(thr_cfg_saved));
	return ret;
#else
	memcpy(&my18b20.coef, &coef_my18b20_def, sizeof(my18b20.coef));
    return NV_ENABLE_PROTECT_ERROR;
#endif
}

nv_sts_t save_config_termostat(void) {
#if NV_ENABLE
	nv_sts_t ret = NV_SUCC;
	if(memcmp(&thr_cfg_saved, &zcl_thermostat_attrs.cfg, sizeof(thr_cfg_saved))) {
		memcpy(&thr_cfg_saved, &zcl_thermostat_attrs.cfg, sizeof(thr_cfg_saved));
		ret = nv_flashWriteNew(1, NV_MODULE_APP,  NV_ITEM_APP_CFG_THERMOSTAT, sizeof(thr_cfg_saved), (uint8_t*)&thr_cfg_saved);
	}
    return ret;
#else
    return NV_ENABLE_PROTECT_ERROR;
#endif
}
#endif // ZCL_THERMOSTAT

#endif // USE_SENSOR_MY18B20

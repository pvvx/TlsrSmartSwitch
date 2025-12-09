#include "app_main.h"
#if USE_BL0942
#include "battery.h"
#include "energy_save.h"
#if USE_SENSOR_MY18B20
#include "my18b20.h"
#endif

extern u64 mul32x32_64(u32 a, u32 b); // hard function (in div_mod.S)

//--------- Work Data --------

#define HEAD            0x55

// rx buffer BL0942
typedef struct __attribute__((packed)) {
    uint8_t     head;
    uint32_t    i_rms       :24;
    uint32_t    v_rms       :24;
    uint32_t    i_fast_rms  :24;
    int32_t     watt        :24;
    uint32_t    cf_cnt      :24;
    uint16_t    freq;
    uint8_t     resv1;
    uint8_t     status;
    uint8_t     resv2;
    uint8_t     resv3;
    uint8_t     crc;
} app_monitoring_t;

// UART RX DMA buffer
typedef struct __attribute__((packed)) {
    volatile u32 dma_len;
    u8 data[32];
} dma_uart_rx_buf_t;

dma_uart_rx_buf_t urxb; // UART RX DMA buffer

app_monitoring_t pkt_buf;

static uint16_t tik_max_current; // count max current, step 1 sec, =0xFFFF - flag end
uint16_t tik_reload, tik_start; // step 1 sec, =0xFFFF - flag end
static uint8_t first_start = true;     // flag

//--------- Data for calculating BL0942 --------


#ifndef BL0942_CURRENT_REF
#define BL0942_CURRENT_REF      16860520 // current x1000: 0..65.535A
#endif
#ifndef BL0942_VOLTAGE_REF
#define BL0942_VOLTAGE_REF      26927694 // voltage x100: 0..655.35V
#endif
// POWER_REF = (2pow32/VOLTAGE_REF)*(2pow32/CURRENT_REF)*353700/305978/73989 = 0.63478593422
#ifndef BL0942_POWER_REF
#define BL0942_POWER_REF        27060025 // poewr x1000: x1000: 0..32.767W, x100 32.767..327.67W, x10: 327.67..3276.7W
#endif
// This REF get from https://github.com/esphome/esphome/blob/dev/esphome/components/bl0942
// this->energy_reference_ = this->power_reference_ * 3600000 / 419430.4
// ENERGY_REF = ((2pow32/POWER_REF)*3600000)/419430.4
#ifndef BL0942_ENERGY_REF
#define BL0942_ENERGY_REF       315272310 // energy x100000
#endif
//
#ifndef BL0942_FREQ_REF
#define BL0942_FREQ_REF         100000000 // (measured: 100175000) x100
#endif

const sensor_pwr_coef_t sensor_pwr_coef_def = {
	    .current = BL0942_CURRENT_REF,
	    .voltage = BL0942_VOLTAGE_REF,
	    .power = BL0942_POWER_REF,
	    .energy = BL0942_ENERGY_REF,
	    .freq = BL0942_FREQ_REF
};

sensor_pwr_coef_t sensor_pwr_coef;
sensor_pwr_coef_t sensor_pwr_coef_saved;

// Remainder from the previous division
typedef struct {
    uint32_t current;
    uint32_t voltage;
    uint32_t power;
    uint32_t energy;
    uint32_t old_energy; // for calculating the energy meter
} old_fract_t;

static old_fract_t old_fract; // Remainder from the previous division

//--------- Loading/Saving Sensor Coefficients --------

nv_sts_t load_config_sensor(void) {
#if NV_ENABLE
	nv_sts_t ret = nv_flashReadNew(1, NV_MODULE_APP,  NV_ITEM_APP_CFG_SENSOR_BL09xx, sizeof(sensor_pwr_coef), (uint8_t*)&sensor_pwr_coef);
	if(ret !=  NV_SUCC) {
		memcpy(&sensor_pwr_coef, &sensor_pwr_coef_def, sizeof(sensor_pwr_coef));
	}
	memcpy(&sensor_pwr_coef_saved, &sensor_pwr_coef, sizeof(sensor_pwr_coef));
	return ret;
#else
    return NV_ENABLE_PROTECT_ERROR;
#endif
}

nv_sts_t save_config_sensor(void) {
#if NV_ENABLE
	nv_sts_t ret = NV_SUCC;
	if(memcmp(&sensor_pwr_coef_saved, &sensor_pwr_coef, sizeof(sensor_pwr_coef))) {
		memcpy(&sensor_pwr_coef_saved, &sensor_pwr_coef, sizeof(sensor_pwr_coef));
		ret = nv_flashWriteNew(1, NV_MODULE_APP,  NV_ITEM_APP_CFG_SENSOR_BL09xx, sizeof(sensor_pwr_coef), (uint8_t*)&sensor_pwr_coef);
	}
    return ret;
#else
    return NV_ENABLE_PROTECT_ERROR;
#endif
}

#if USE_CALIBRATE_CVP
//--------- Data for calibrate calculating BL09377 --------

typedef struct {
    uint32_t current;
    uint32_t voltage;
    uint32_t power;
    uint8_t cnt;
} reg_calibrate_t;

static reg_calibrate_t reg_calibrate;

static void sensor_calibrate_coef(void);

zcl_sensor_calibrate_t sensor_calibrate;

/* cnt max 0x1ffffff
 return: fix_point(9.23) = x / cnt */
static uint32_t _calk_coef(uint32_t cnt, uint16_t x) {
	uint32_t tmp, fract, val;
	tmp = x << 16;
	val = tmp / cnt;
	fract = tmp - (val * cnt);
	val <<= 7;
	fract <<= 7;
	fract /= cnt;
	val += fract;
	return val;
}

/* Sensor Calibrate Coefficients */
static void sensor_calibrate_coef(void) {
	bool save_flg = false;
	reg_calibrate.cnt = 0;
	if(sensor_calibrate.current) { // in 0.001 A, max 25.000A
		if(reg_calibrate.current) { // x4
			// coef.current - fp(0.32)
			sensor_pwr_coef.current =
				_calk_coef(reg_calibrate.current, sensor_calibrate.current) << (32+2-23); // << 11
			save_flg = true;
		}
		sensor_calibrate.current = 0;
	}
	if(sensor_calibrate.voltage) { // in 0.01V, max 300.00V
		if(reg_calibrate.voltage) { // x4
			// coef.voltage - fp(0.32)
			sensor_pwr_coef.voltage =
				_calk_coef(reg_calibrate.voltage, sensor_calibrate.voltage) << (32+2-23); // << 11
			save_flg = true;
		}
		sensor_calibrate.voltage = 0;
	}
	if(sensor_calibrate.power) { // in 0.1 W, max 6250.0W (250V*25A)
		if(sensor_calibrate.power == 1) {
			sensor_pwr_coef.power = mul32x32_64(sensor_pwr_coef.current, sensor_pwr_coef.voltage) >> 24;
		} else if(reg_calibrate.power) { // x4
			// coef.power - fp(8.24)
			sensor_pwr_coef.power =
				_calk_coef(reg_calibrate.power, sensor_calibrate.power) << (24+2-23); // << 3
		}
		// energy = power * 3600000 / 419430.4
		// 2pow32/(419430.4*100/3600000) = 368640000
		sensor_pwr_coef.energy = mul32x32_64(sensor_pwr_coef.power, 368640000) >> 32;
		save_flg = true;
		sensor_calibrate.power = 0;
	}
	if(save_flg) {
		save_config_sensor();
	}
}

void check_start_calibrate(void) {
	reg_calibrate.current = 0;
	reg_calibrate.voltage = 0;
	reg_calibrate.power = 0;
	reg_calibrate.cnt = 1;
}

#endif

//--------------------------------

// Chk BL0942
static uint8_t checksum(uint8_t *data, uint16_t length) {
    uint8_t crc8 = 0;
    for(uint8_t i = 0; i < (length - 1); i++) {
        crc8 += data[i];
    }
    crc8 += 0x58;
    return ~crc8;
}

//-----------------------
// UART 4800 BAUD
#define UART_BAUD 4800 // bps
#if CLOCK_SYS_CLOCK_HZ == 16000000
#define uartCLKdiv 475 // 16000000/(6+1)/(475+1) = 4801.92
#define bwpc 6
#elif CLOCK_SYS_CLOCK_HZ == 24000000
#define uartCLKdiv 999 // 24000000/(4+1)/(999+1) = 4800
#define bwpc 4
#elif CLOCK_SYS_CLOCK_HZ == 32000000
#define uartCLKdiv 832 // 32000000/(7+1)/(832+1) = 4796.163
#define bwpc 7
#elif CLOCK_SYS_CLOCK_HZ == 48000000
#define uartCLKdiv 999 // 48000000/(9+1)/(999+1) = 4800
#define bwpc 9
#endif

// Initializing UART for BL0942
void app_sensor_init(void) {
	load_config_sensor();
	load_config_min_max();
	energy_restore();
	if(!config_min_max.time_start)
	    tik_start = 0xffff;
	if(!config_min_max.time_reload)
		tik_reload = 0xffff;
	// tik_max_current = 0;
	if(!dev_gpios.tx)
		dev_gpios.tx = GPIO_UART_TX;
	if(!dev_gpios.rx)
		dev_gpios.rx = GPIO_UART_RX;
    drv_uart_pin_set(dev_gpios.tx, dev_gpios.rx);

    reg_clk_en0 |= FLD_CLK0_UART_EN; // Enable CLK UART

    // reg_clk_en1 |= FLD_CLK1_DMA_EN;

    uart_reset();

    uart_recbuff_init((unsigned char *)&urxb, sizeof(urxb.data));

    reg_dma0_addr = (uint16_t)((uint32_t)(&urxb)); //set receive buffer address
    reg_dma0_size = sizeof(urxb.data)>>4; //set receive buffer size
    reg_dma0_mode = FLD_DMA_WR_MEM;   //set DMA 0 mode to 0x01 for receive
    reg_dma0_addrHi = 0x04;

    // set reg_uart_clk_div/reg_uart_ctrl0
    REG_ADDR32(0x094) = MASK_VAL(FLD_UART_CLK_DIV, uartCLKdiv, FLD_UART_CLK_DIV_EN, 1)
        | ((MASK_VAL(FLD_UART_BPWC, bwpc) | (FLD_UART_RX_DMA_EN)) << 16) // set bit width, enable UART RX DMA mode
        | ((MASK_VAL(FLD_UART_CTRL1_STOP_BIT, 0)) << 24) // 00: 1 bit, 01: 1.5bit 1x: 2bits;
        ;
    reg_dma_chn_en |= FLD_DMA_CHN_UART_RX;

    TL_ZB_TIMER_SCHEDULE(app_monitoringCb, NULL, TIMEOUT_1SEC);
    TL_ZB_TIMER_SCHEDULE(energy_timerCb, NULL, TIMEOUT_1MIN);
}


// Task BL0942
void monitoring_handler(void) {

    int32_t  power;
    uint32_t current, voltage, energy, freq;
    uint64_t tmp;

    app_monitoring_t *pkt = (app_monitoring_t*)urxb.data;
    if(reg_dma_rx_rdy0 & FLD_DMA_IRQ_UART_RX) { // new data ?
        reg_uart_status0 |= FLD_UART_CLEAR_RX_FLAG | FLD_UART_RX_ERR_FLAG;
        if(urxb.dma_len == sizeof(app_monitoring_t)
          && pkt->head == HEAD
          && checksum((uint8_t *)pkt, sizeof(app_monitoring_t)) == pkt->crc) {
        	memcpy(&pkt_buf, pkt, sizeof(pkt_buf));
        	pkt = &pkt_buf;
            reg_dma_rx_rdy0 = FLD_DMA_IRQ_UART_RX;

            energy = pkt->cf_cnt;
        	if (first_start) {
                first_start = false;
            	old_fract.old_energy = energy;
            } else {

                current = pkt->i_rms;
                voltage = pkt->v_rms;
                power = pkt->watt;
                freq = pkt->freq;

                if(power < 0)
                    power = -power;

#if USE_CALIBRATE_CVP
                if(reg_calibrate.cnt) {
                	reg_calibrate.cnt++;
                	reg_calibrate.current += current;
                	reg_calibrate.voltage += voltage;
                	reg_calibrate.power += power;
                	if(reg_calibrate.cnt > 4) {
                		sensor_calibrate_coef();
                	}
                }
#endif
                tmp = mul32x32_64(current, sensor_pwr_coef.current);
                tmp += old_fract.current;
                old_fract.current = tmp & 0xffffffff;
                current = tmp >> 32;
                g_zcl_msAttrs.current = (uint16_t)current;

                tmp = mul32x32_64(voltage, sensor_pwr_coef.voltage);
                tmp += old_fract.voltage;
                old_fract.voltage = tmp & 0xffffffff;
                voltage = tmp >> 32;
                g_zcl_msAttrs.voltage = (uint16_t)voltage;

                // power x1000 0..3276.750W

                tmp = mul32x32_64(power, sensor_pwr_coef.power);
                tmp += old_fract.power;
                old_fract.power = tmp & 0xffffff;
                power = tmp >> 24;

                if(power > 32767000) { // (max 32767.000W)
                	power = 32767;
                	g_zcl_msAttrs.power_divisor = 1;
                } else if(power > 3276700) {
                    // x1: 3276.7..32767W
                    power += 500;
                    power /= 1000;
                    g_zcl_msAttrs.power_divisor = 1;
                } else if(power > 327670) {
                    // x10: 327.6..3276.7W
                    power += 50;
                    power /= 100;
                    g_zcl_msAttrs.power_divisor = 10;
                } else if(power > 32767) {
                    // x100 32.767..327.67W
                    power += 5;
                    power /= 10;
                    g_zcl_msAttrs.power_divisor = 100;
                } else {
                    // x1000: x1000: 0..32.767W
                    g_zcl_msAttrs.power_divisor = 1000;
                }
                g_zcl_msAttrs.power = (int16_t)power;

                freq = (sensor_pwr_coef.freq + (freq >> 1))/ freq;

                g_zcl_msAttrs.freq = (uint16_t)freq;

                freq =  energy; // save energy
                if(energy < old_fract.old_energy) {
                	energy += 0x1000000 - old_fract.old_energy; // 24 bits
                } else {
                	energy -= old_fract.old_energy;
                }
                old_fract.old_energy = freq; // restore energy

               	tmp = mul32x32_64(energy, sensor_pwr_coef.energy);
                tmp += old_fract.energy;
                old_fract.energy = tmp & 0xffffff; // 24 bits
                energy = tmp >> 24;

              	if(energy) {
               		g_zcl_seAttrs.cur_sum_delivered += energy;
               		new_save_data = true; // energy_save();
               	}

              	//TODO: Calculate Power factor = ?

                if(config_min_max.min_voltage && voltage < config_min_max.min_voltage) {
                	tik_reload = 0;
            		if(tik_start != 0xffff) { // startup timeout expired?
            			tik_start = 0; // continue from the beginning startup timeout
            		}
               		if (config_min_max.emergency_off & BIT(BIT_MIN_VOLTAGE_OFF)) {
            			relay_off |= BIT(BIT_MIN_VOLTAGE_OFF);
               		}
                } else if(config_min_max.max_voltage && voltage > config_min_max.max_voltage) {
                	tik_reload = 0; // continue the reload timeout count from the beginning, relay Off
            		if(tik_start != 0xffff) { // startup timeout expired?
            			tik_start = 0; // continue from the beginning startup timeout, relay Off
            		}
               		if (config_min_max.emergency_off & BIT(BIT_MAX_VOLTAGE_OFF)) {
            			relay_off |= BIT(BIT_MAX_VOLTAGE_OFF);
               		}
                } else if(config_min_max.max_current
                  && config_min_max.time_max_current
                  && (current > config_min_max.max_current)) {
            		if(tik_max_current != 0xffff) { // Over Current timeout expired?
            			tik_max_current += 8;
            			if(tik_max_current >= config_min_max.time_max_current) {
            				tik_reload = 0; // continue the reload timeout count from the beginning, relay Off
            				tik_max_current = 0xffff; // Over Current timeout expired
            				if (config_min_max.emergency_off & BIT(BIT_MAX_CURRENT_OFF))
            					relay_off |= BIT(BIT_MAX_CURRENT_OFF);
            			}
            		} else { // Over Current timeout expired
            			tik_reload = 0; // continue the reload timeout count from the beginning, relay Off
            		}
                } else { // all ok
                	tik_max_current = 0;
                	if(tik_start >= config_min_max.time_start) {
                		tik_start = 0xffff;
                	}
                	if(tik_reload >= config_min_max.time_reload) {
                		tik_reload = 0xffff;
                	}
#if USE_THERMOSTAT // USE_SENSOR_MY18B20
               		set_therm_relay_status(cfg_on_off.onOff);
#else
               		set_relay_status(cfg_on_off.onOff);
#endif
                	return;
                }
            	set_relay_status(0); // Relay Off
            }
    	} else {
    		reg_dma_rx_rdy0 = FLD_DMA_IRQ_UART_RX;
    	}
    }
}

// Step 1 sec
int32_t app_monitoringCb(void *arg) {

    REG_ADDR16(0x90) = 0xAA58; // Send cmd: "Read full packet"
	if(tik_reload != 0xffff)
		tik_reload++;
	if(tik_start != 0xffff)
		tik_start++;
#if USE_SENSOR_MY18B20
	my18b20.start_measure = 1;
#endif
    return 0;
}


#endif // USE_BL0942

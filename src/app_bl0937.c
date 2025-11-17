#include "app_main.h"
#if USE_BL0937
#include "energy_save.h"
#if USE_SENSOR_MY18B20
#include "my18b20.h"
#endif

extern u64 mul32x32_64(u32 a, u32 b); // hard function (in div_mod.S)

//--------- Work Data --------

static uint16_t tik_max_current; // count max current, step 8 sec, =0xFFFF - flag end
uint16_t tik_reload, tik_start; // step 1 sec, =0xFFFF - flag end


//--------- Data for calculating BL09377 --------

// Pulse counters BL0937
typedef struct {
    uint32_t cnt_current;
    uint32_t cnt_voltage;
    uint32_t cnt_power;
    uint32_t cnt_sel;
} bl0937_cnt_t;

static bl0937_cnt_t bl0937_cnt; // Pulse counters BL0937

// Remainder from the previous division
typedef struct {
    uint32_t current;
    uint32_t voltage;
    uint32_t power;
    uint32_t energy;
} old_fract_t;

static old_fract_t old_fract; // Remainder from the previous division

/*
 * Test2:
 * ~6.430A    575 Hz x4 2300*2.795652173913043478 = 6429.9999999999999994 : 183216
 * ~215.00 V 1714 Hz x4 6856*3.135939323220536756126 = 21499.999999999999999999856 : 205517
 * ~1415.00W 1054 Hz x8 8432*16.781309297912713472485 = 141499.99999999999999999352 : 1099780
 * ~1400.00W 8360 tick, 16.7464114832535885167464*2pow16 = 1097492.8229665071770334920704 : 1097493
 */
// Sensor Coefficients (for period 8 sec)
#define BL0937_CURRENT_REF        (178060) 		// x1000: 0..65.535A (divisor = 1000 - > A)
#define BL0937_VOLTAGE_REF        (197732)  	// x100: 0..655.35V (divisor = 100 - > V)
#define BL0937_POWER_REF          (1081452) 	// x100 0..327.67W, x10: 327.67..3276.7W (divisor = 10, 100 - > W)
#define BL0937_ENERGY_REF         ((BL0937_POWER_REF + 225)/450) //(=2403) x100 Wh (divisor = 100000 - > kWh)

const sensor_pwr_coef_t sensor_pwr_coef_def = {
	    .current = BL0937_CURRENT_REF,
	    .voltage = BL0937_VOLTAGE_REF,
	    .power = BL0937_POWER_REF,
	    .energy = BL0937_ENERGY_REF
};

sensor_pwr_coef_t sensor_pwr_coef;
sensor_pwr_coef_t sensor_pwr_coef_saved;

//--------- Initialization --------

// Initializing Timers1 for BL0937
static void timer1_gpio_init(GPIO_PinTypeDef pin,GPIO_PolTypeDef pol)
{
	unsigned char bit = pin & 0xff;

	gpio_set_func(pin, AS_GPIO);
	gpio_set_output_en(pin, 0); //disable output
	gpio_set_input_en(pin, 1);//enable input
	gpio_setup_up_down_resistor(pin, PM_PIN_PULLUP_1M);

	BM_SET(reg_gpio_irq_risc1_en(pin), bit);

	if(pol==POL_FALLING)
	{
		BM_SET(reg_gpio_pol(pin), bit);
	}
	else if(pol==POL_RISING)
	{
		BM_CLR(reg_gpio_pol(pin), bit);
	}
	reg_irq_mask |= FLD_IRQ_TMR1_EN;
	reg_tmr1_tick = 0;
	reg_tmr1_capt = 0xffffffff;
	reg_tmr_sta = FLD_TMR_STA_TMR1; //clear irq status
	reg_tmr_ctrl &= (~FLD_TMR1_MODE);
	reg_tmr_ctrl |= (TIMER_MODE_GPIO_TRIGGER<<4);
	reg_tmr_ctrl |= FLD_TMR1_EN;
}

// Initializing Timers2 for BL0937
static void timer2_gpio_init(GPIO_PinTypeDef pin,GPIO_PolTypeDef pol)
{
	unsigned char bit = pin & 0xff;

	gpio_set_func(pin, AS_GPIO);
	gpio_set_output_en(pin, 0); //disable output
	gpio_set_input_en(pin, 1);//enable input
	gpio_setup_up_down_resistor(pin, PM_PIN_PULLUP_1M);

	BM_SET(reg_gpio_irq_risc2_en(pin), bit);

	if(pol==POL_FALLING)
	{
		BM_SET(reg_gpio_pol(pin), bit);
	}
	else if(pol==POL_RISING)
	{
		BM_CLR(reg_gpio_pol(pin), bit);
	}
	reg_irq_mask |= FLD_IRQ_TMR2_EN;
	reg_tmr2_tick = 0;
	reg_tmr2_capt = 0xffffffff;
	reg_tmr_sta = FLD_TMR_STA_TMR2; //clear irq status
	reg_tmr_ctrl &= (~FLD_TMR2_MODE);
	reg_tmr_ctrl |= (TIMER_MODE_GPIO_TRIGGER<<7);
	reg_tmr_ctrl |= FLD_TMR2_EN;
}

// Initializing Timers1,2 for BL0937
void app_sensor_init(void) {
	load_config_sensor();
	load_config_min_max();
	energy_restore();
	if(!config_min_max.time_start)
	    tik_start = 0xffff;
	if(!config_min_max.time_reload)
		tik_reload = 0xffff;
	//bl0937_cnt.cnt_sel = 0;
	gpio_write(GPIO_SEL, 0);
	timer1_gpio_init(GPIO_CF, POL_RISING);
	timer2_gpio_init(GPIO_CF1, POL_RISING);
	TL_ZB_TIMER_SCHEDULE(app_monitoringCb, NULL, TIMEOUT_1SEC);
    TL_ZB_TIMER_SCHEDULE(energy_timerCb, NULL, TIMEOUT_1MIN);
}

//--------- Monitoring --------

// Calculation of variables
void bl0937_new_dataCb(void *args) {

	uint32_t  current, voltage, power, energy;
    uint64_t tmp;

	current = bl0937_cnt.cnt_current;
	bl0937_cnt.cnt_current = 0;
    voltage = bl0937_cnt.cnt_voltage;
    bl0937_cnt.cnt_voltage = 0;
    power = bl0937_cnt.cnt_power;
    bl0937_cnt.cnt_power = 0;

	current *= sensor_pwr_coef.current;
    current += old_fract.current;
    old_fract.current = current & 0xffff;
    current >>= 16;
    g_zcl_msAttrs.current = (uint16_t)current; // (max 32.767A)

    voltage *= sensor_pwr_coef.voltage;
    voltage += old_fract.voltage;
    old_fract.voltage = voltage & 0xffff;
    voltage >>= 16;
    g_zcl_msAttrs.voltage = (uint16_t)voltage; // (max 327.67V)


    energy = power;

   	tmp = mul32x32_64(power, sensor_pwr_coef.power);
    tmp += old_fract.power;
    old_fract.power = tmp & 0xffff;
    power = tmp >> 16;

   	if(power >= 3276700) {
   		power = 32767;
        energy = 7282;
        g_zcl_msAttrs.power_divisor = 1;
   	} else {
   		if(power > 327670) {
   	        // x10: 3276.7..32767W (max 149A 220V)
   	        power += 50;
   	        power /= 100;
   	        g_zcl_msAttrs.power_divisor = 1;
   	    } else if(power > 32767) {
   	        // x10: 327.6..3276.7W (max 14.9A 220V)
   	        power += 5;
   	        power /= 10;
   	        g_zcl_msAttrs.power_divisor = 10;
   	    } else {
   	        // x100 0..327.67W (max 1.49A 220V)
   	        g_zcl_msAttrs.power_divisor = 100;
   	    }
   		g_zcl_msAttrs.power = (int16_t)power;

   		tmp = mul32x32_64(energy, sensor_pwr_coef.energy);
   		tmp += old_fract.energy;
   		old_fract.energy = tmp & 0xffff;
   		energy = tmp >> 16;
   	}

   	if(energy) {
		save_data.energy += energy;
		g_zcl_seAttrs.cur_sum_delivered = save_data.energy;
        new_save_data = true; // energy_save();
	}

	//TODO: Calculate Power factor = ?

    if((config_min_max.min_voltage && voltage < config_min_max.min_voltage)
    	|| (config_min_max.max_voltage && voltage > config_min_max.max_voltage)) {
    	tik_reload = 0;
		if(tik_start != 0xffff)
			tik_start = 0;
    	if(relay_settings.status_onoff[0])
    		//cmdOnOff_off(dev_relay.unit_relay[0].ep);
    		set_relay_status(0, 0);
    	return;
    }
    if(config_min_max.max_current
      && config_min_max.time_max_current
      && (current > config_min_max.max_current)) {
		tik_reload = 0;
		if(tik_start != 0xffff)
			tik_start = 0;
		if(relay_settings.status_onoff[0]) {
			if(tik_max_current == 0xffff)
				return;
			tik_max_current += 8;
        	if(tik_max_current >= config_min_max.time_max_current) {
        		tik_max_current = 0xffff;
        		//cmdOnOff_off(dev_relay.unit_relay[0].ep);
        		set_relay_status(0, 0);
            	return;
        	}
		}
    } else {
    	tik_max_current = 0;
    }
    if(config_min_max.time_start && tik_start >= config_min_max.time_start)
    	tik_start = 0xffff;
    if(config_min_max.time_reload && tik_reload >= config_min_max.time_reload) {
    	tik_reload = 0xffff;
    	if(relay_settings.status_onoff[0]) {
    		// cmdOnOff_on(dev_relay.unit_relay[0].ep);
    		set_relay_status(0, 1);
    	}
    }
}

// Step 1 sec
// Data collection from BL0937
int32_t app_monitoringCb(void *arg) {

	uint32_t reg;
	uint8_t r;

	bl0937_cnt.cnt_sel++; // next sec

	r = irq_disable();
	reg_tmr_ctrl &= ~(FLD_TMR2_EN); // stop timer
	reg = reg_tmr2_tick; // get count

	if(bl0937_cnt.cnt_sel & 1) {
		gpio_write(GPIO_SEL, 1);
		bl0937_cnt.cnt_current += reg; // add count current
	} else {
		gpio_write(GPIO_SEL, 0);
		bl0937_cnt.cnt_voltage += reg; // add count voltage
	}

	reg_tmr2_tick = 0; // clear timer count
	reg_tmr_sta = FLD_TMR_STA_TMR2; //clear irq status
	reg_tmr_ctrl |= FLD_TMR2_EN; // start timer
	irq_restore(r);


	if((bl0937_cnt.cnt_sel & 7) == 0) {
		r = irq_disable();
		reg_tmr_ctrl &= ~(FLD_TMR1_EN); // stop timer
		bl0937_cnt.cnt_power = reg_tmr1_tick; // get count
		reg_tmr1_tick = 0;	// clear timer count
		reg_tmr_sta = FLD_TMR_STA_TMR1; //clear irq status
		reg_tmr_ctrl |= FLD_TMR1_EN; // start timer
		irq_restore(r);
		TL_SCHEDULE_TASK(bl0937_new_dataCb, NULL);
	}

	if(tik_reload != 0xffff)
		tik_reload++;
	if(tik_start != 0xffff)
		tik_start++;
#if USE_SENSOR_MY18B20
	my18b20.start_measure = 1;
#endif
	return 0;
}

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
		sensor_pwr_coef.energy = mul32x32_64(sensor_pwr_coef.power+220, 9544372) >> 32; // 0x100000000/(60*60/8)=9544371.76888
		memcpy(&sensor_pwr_coef_saved, &sensor_pwr_coef, sizeof(sensor_pwr_coef));
		ret = nv_flashWriteNew(1, NV_MODULE_APP,  NV_ITEM_APP_CFG_SENSOR_BL09xx, sizeof(sensor_pwr_coef), (uint8_t*)&sensor_pwr_coef);
	}
    return ret;
#else
    return NV_ENABLE_PROTECT_ERROR;
#endif
}




#endif // USE_BL0937

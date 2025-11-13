#include "app_main.h"
#if USE_BL0937
#include "battery.h"

//------- Save Energy ----

// Address save energy count to Flash
typedef struct {
    uint32_t flash_addr_save;          /* flash page address saved */
    uint32_t flash_addr_start;         /* flash page address start */
    uint32_t flash_addr_end;           /* flash page address end   */
} energy_cons_t;

static energy_cons_t energy_cons;

// Save energy count in Flash
typedef struct {
    uint64_t energy;
    uint64_t xor_energy;
} save_data_t;

static save_data_t save_data;

//--------- Work Data --------

static uint8_t  first_start = true;     // flag
static bool new_save_data = false;      // flag

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
 * Test Lamp: 100W ~229.5V 432 mA (period 1+1+1+1 sec)
 * cnt_voltage = 3780
 * (22950<<16)/3780=397897.14285714285714285714285714285714285714285714285714285714285714285714285714286
 * cnt_current = 79.5
 * (4320<<16)/795=356120.15094339622641509433962264150943396226415094339622641509433962264150943396226
 * cnt_power = 303
 * (10000<<16)/303=2162904.29042904290429042904290429042904290429042904290429042904290429042904290429043
 * energy (=power/450):
 * 2162904.29/(60*60/8)=4806.453977777777777777777777777777777777777777777777777777777777777777777777778
 *
 * TODO: user set koef?
 */
// Coefficient for period 8 sec
#define BL0937_POWER_REF          (2162904/2) 	// x100 0..327.67W, x10: 327.67..3276.7W (divisor = 10, 100 - > V)
#define BL0937_VOLTAGE_REF        (395464/2)  	// x100: 0..655.35V (divisor = 100 - > V)
#define BL0937_CURRENT_REF        (356120/2) 	// x1000: 0..65.535A (divisor = 1000 - > A)
#define BL0937_ENERGY_REF         (4806/2) 	    // x100 Wh (divisor = 100000 - > kWh)

// Coefficient
typedef struct {
    uint32_t current;
    uint32_t voltage;
    uint32_t power;
    uint32_t energy;
} bl0937_coef_t;

const bl0937_coef_t bl0937_coef = {
	    .current = BL0937_CURRENT_REF,
	    .voltage = BL0937_VOLTAGE_REF,
	    .power = BL0937_POWER_REF,
	    .energy = BL0937_ENERGY_REF
};

//--------- Initialization --------

// Initializing Timers1 for BL0937
static void timer1_gpio_init(GPIO_PinTypeDef pin,GPIO_PolTypeDef pol)
{
	gpio_set_func(pin ,AS_GPIO);
	gpio_set_output_en(pin, 0); //disable output
	gpio_set_input_en(pin ,1);//enable input
/*
	if(pol==POL_FALLING)
	{
		gpio_setup_up_down_resistor(pin,PM_PIN_PULLUP_10K);
	}
	else if(pol==POL_RISING)
	{
		gpio_setup_up_down_resistor(pin,PM_PIN_PULLDOWN_100K);
	}
*/
	unsigned char bit = pin & 0xff;

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
	reg_tmr1_capt = 0xfffff;
	reg_tmr_sta = FLD_TMR_STA_TMR1; //clear irq status
	reg_tmr_ctrl &= (~FLD_TMR1_MODE);
	reg_tmr_ctrl |= (TIMER_MODE_GPIO_TRIGGER<<4);
	reg_tmr_ctrl |= FLD_TMR1_EN;
}

// Initializing Timers2 for BL0937
static void timer2_gpio_init(GPIO_PinTypeDef pin,GPIO_PolTypeDef pol)
{
	gpio_set_func(pin ,AS_GPIO);
	gpio_set_output_en(pin, 0); //disable output
	gpio_set_input_en(pin ,1);//enable input
/*
	if(pol==POL_FALLING)
	{
		gpio_setup_up_down_resistor(pin,PM_PIN_PULLUP_10K);
	}
	else if(pol==POL_RISING)
	{
		gpio_setup_up_down_resistor(pin,PM_PIN_PULLDOWN_100K);
	}
*/
	unsigned char bit = pin & 0xff;

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
	reg_tmr2_capt = 0xfffff;
	reg_tmr_sta = FLD_TMR_STA_TMR2; //clear irq status
	reg_tmr_ctrl &= (~FLD_TMR2_MODE);
	reg_tmr_ctrl |= (TIMER_MODE_GPIO_TRIGGER<<7);
	reg_tmr_ctrl |= FLD_TMR2_EN;
}

// Initializing Timers1,2 for BL0937
void app_sensor_init(void) {
	timer1_gpio_init(GPIO_CF, POL_RISING);
	timer2_gpio_init(GPIO_CF1, POL_RISING);
}

//--------- Monitoring --------

// Calculation of variables
void bl0937_new_dataCb(void *args) {

	uint32_t  current, voltage, power, energy;

	current = bl0937_cnt.cnt_current;
	bl0937_cnt.cnt_current = 0;
    voltage = bl0937_cnt.cnt_voltage;
    bl0937_cnt.cnt_voltage = 0;
    power = bl0937_cnt.cnt_power;

    if (first_start) {
        first_start = false;
    } else {
        current *= bl0937_coef.current;
        current += old_fract.current;
        old_fract.current = current & 0xffff;
        current >>= 16;
        g_zcl_msAttrs.current = (uint16_t)current;

        voltage *= bl0937_coef.voltage;
        voltage += old_fract.voltage;
        old_fract.voltage = voltage & 0xffff;
        voltage >>= 16;
        g_zcl_msAttrs.voltage = (uint16_t)voltage;


        energy = power;

        power *= bl0937_coef.power;
        power += old_fract.power;
        old_fract.power = power & 0xffff;
        power >>= 16;

        if(power > 327670) {
            // x10: 327.6..3276.7W
            power += 5;
            power /= 10;
            g_zcl_msAttrs.power_divisor = 10;
        } else {
            // x100 0..327.67W
            g_zcl_msAttrs.power_divisor = 100;
        }
        g_zcl_msAttrs.power = (int16_t)power;

        energy *= bl0937_coef.energy;
        energy += old_fract.energy;
        old_fract.energy = energy & 0xffff;
        energy >>= 16;
        save_data.energy += energy;
        g_zcl_seAttrs.cur_sum_delivered = save_data.energy;

        //TODO: Calculate Power factor = ?

        new_save_data = true; // energy_save();
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

	return 0;
}

//--------- Save Energy --------

// Clearing USER_DATA
static void clear_user_data(void) {

    uint32_t flash_addr = energy_cons.flash_addr_start;
    while(flash_addr < energy_cons.flash_addr_end) {
        flash_erase_sector(flash_addr);
        flash_addr += FLASH_SECTOR_SIZE;
    }
}

// Saving the energy meter
static void save_dataCb(void *args) {

	battery_detect(0);

    if (save_data.xor_energy != ~save_data.energy) {

        save_data.xor_energy = ~save_data.energy;

        light_blink_start(1, 250, 250);

        if (energy_cons.flash_addr_save == energy_cons.flash_addr_end) {
            energy_cons.flash_addr_save = energy_cons.flash_addr_start;
        }
        if ((energy_cons.flash_addr_save & (FLASH_SECTOR_SIZE-1)) == 0) {
            flash_erase(energy_cons.flash_addr_save);
        }

        flash_write(energy_cons.flash_addr_save, sizeof(save_data), (uint8_t*)&save_data);

        energy_cons.flash_addr_save += sizeof(save_data);
    }
}

// Initializing USER_DATA storage addresses in Flash memory
static void init_save_addr_drv(void) {

	u32 mid = flash_read_mid();
	mid >>= 16;
	mid &= 0xff;
	if(mid >= FLASH_SIZE_1M) {
        energy_cons.flash_addr_start = BEGIN_USER_DATA_F1M;
        energy_cons.flash_addr_end = END_USER_DATA_F1M;
    } else {
        energy_cons.flash_addr_start = BEGIN_USER_DATA_F512K;
        energy_cons.flash_addr_end = END_USER_DATA_F512K;
    }
    energy_cons.flash_addr_save = energy_cons.flash_addr_start;
    save_data.energy = 0;
    save_data.xor_energy = -1;
    g_zcl_seAttrs.cur_sum_delivered = 0;
}


// Read & check valid blk (Save Energy)
static int check_saved_blk(uint32_t flash_addr, save_data_t * pdata) {
    if(flash_addr >= energy_cons.flash_addr_end)
        flash_addr = energy_cons.flash_addr_start;
    flash_read_page(flash_addr, sizeof(save_data_t), (uint8_t*)pdata);
    if(pdata->energy == -1 && pdata->xor_energy == -1) {
        return -1;
    } else if(pdata->energy == ~pdata->xor_energy) {
        return 0;
    }
    return 1;
}

// Start initialize (Save Energy)
int energy_restore(void) {
    int ret;
    uint32_t flash_addr;

    save_data_t tst_data;

    init_save_addr_drv();

    flash_addr = energy_cons.flash_addr_start;

    while(flash_addr < energy_cons.flash_addr_end) {
        flash_addr &= ~(FLASH_SECTOR_SIZE-1);
        ret = check_saved_blk(flash_addr, &tst_data);
        if(ret < 0) {
            flash_addr += FLASH_SECTOR_SIZE;
            continue;
        }
        if(ret == 0) {
            memcpy(&save_data, &tst_data, sizeof(save_data)); // save_data = tst_data;
            if((check_saved_blk(flash_addr + FLASH_SECTOR_SIZE, &tst_data) == 0)
            && tst_data.energy > save_data.energy) {
                flash_addr += FLASH_SECTOR_SIZE;
                continue;
            }
        }
        flash_addr += sizeof(tst_data);
        while(flash_addr < energy_cons.flash_addr_end) {
            ret = check_saved_blk(flash_addr, &tst_data);
            if(ret == 0) {
                if(tst_data.energy > save_data.energy) {
                    memcpy(&save_data, &tst_data, sizeof(save_data)); // save_data = tst_data;
                } else {
                    flash_addr &= ~(FLASH_SECTOR_SIZE-1);
                    energy_cons.flash_addr_save = flash_addr;
                    g_zcl_seAttrs.cur_sum_delivered = save_data.energy;
                    return 0;
                }
            } else if(ret < 0) {
                energy_cons.flash_addr_save = flash_addr;
                g_zcl_seAttrs.cur_sum_delivered = save_data.energy;
                return 0;
            }
            flash_addr += sizeof(tst_data);
        }
    }
    return 1;
}

// Clear all USER_DATA (Save Energy)
void energy_remove(void) {
    init_save_addr_drv();
    clear_user_data();
}

// Step 1 minutes (Save Energy)
int32_t energy_timerCb(void *args) {

    if (new_save_data) {
        new_save_data = false;
        TL_SCHEDULE_TASK(save_dataCb, NULL);
    }
    return 0;
}


#endif // USE_BL0937

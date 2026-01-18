#include "app_main.h"
#include "sensors.h"

typedef struct {
	uint32_t id[2];
	dev_gpios_t gpios;
} ext_tab_gpios_t;

typedef struct {
	uint32_t id[2];
	dev_gpios_t gpios;
	uint32_t crc;
} flash_tab_gpios_t;

const ext_tab_gpios_t  tab_gpios = {
	.id = { 0x5F424154, 0x4F495047 }, // "TAB_GPIO"
	.gpios = {
		.rl = GPIO_RELAY1,
		.led1 = GPIO_LED1,
		.led2 = GPIO_LED2,
		.key = GPIO_BUTTON,
		.sw1 = GPIO_SWITCH1,
		.swire = GPIO_ONEWIRE1,
#if USE_BL0937
		.sel = GPIO_SEL,
		.cf = GPIO_CF,
		.cf1 = GPIO_CF1,
#endif
#if USE_BL0942
		.rx = GPIO_UART_RX,
		.tx = GPIO_UART_TX,
#endif
	}
};

dev_gpios_t  dev_gpios;
uint8_t relay_off, relay_state;

static void check_first_start(void) {
    switch(cfg_on_off.startUpOnOff) {
        case ZCL_START_UP_ONOFF_SET_ONOFF_TO_PREVIOUS:
            if (cfg_on_off.onOff)
            	cmdOnOff_on();
            else
            	cmdOnOff_off();
            break;
        case ZCL_START_UP_ONOFF_SET_ONOFF_TOGGLE:
            cmdOnOff_toggle();
            break;
        case ZCL_START_UP_ONOFF_SET_ONOFF_TO_ON:
            cmdOnOff_on();
            break;
        case ZCL_START_UP_ONOFF_SET_ONOFF_TO_OFF:
            cmdOnOff_off();
            break;
        default:
            cmdOnOff_off();
            break;
    }
}

/* Get relay on/off */
bool get_relay_status(void) {
#if RELAY_ON
    return gpio_read(dev_gpios.rl)? true : false;
#else
    return gpio_read(dev_gpios.rl)? false : true;
#endif
}

/* Set relay and led, if not emergency */
void set_relay_status(bool status) {
#if USE_METERING
	if(relay_off || tik_reload != 0xffff || tik_start != 0xffff)
		status = false;
#endif
	if(status)
		light_on();
	else
		light_off();
#if RELAY_ON
	gpio_write(dev_gpios.rl, status);
#else
	gpio_write(dev_gpios.rl, !status);
#endif
#if !USE_SWITCH
	if(relay_state != status) {
		if(status)
		{
#if RELAY_ON
			remoteCmdOnOff(ZCL_CMD_ONOFF_ON);
#else
			remoteCmdOnOff(ZCL_CMD_ONOFF_OFF);
#endif
		} else {
#if RELAY_ON
			remoteCmdOnOff(ZCL_CMD_ONOFF_OFF);
#else
			remoteCmdOnOff(ZCL_CMD_ONOFF_ON);
#endif
		}
	}
#endif // !USE_SWITCH
	relay_state = status;
}

#if USE_THERMOSTAT // USE_SENSOR_MY18B20
/* if TERMOSTAT On - Not set relay,
   if TERMOSTAT Off - set relay */
void set_therm_relay_status(bool status) {
 #ifdef ZCL_THERMOSTAT
  		if(zcl_thermostat_attrs.cfg.sys_mode == TH_SMODE_OFF)
  			set_relay_status(status);
 #endif
}
#endif

void gpio_input_init(GPIO_PinTypeDef pin, GPIO_PullTypeDef pulup) {
	gpio_set_func(pin, AS_GPIO); //set pin gpio
	gpio_set_output_en(pin, 0); //disable output
	gpio_set_input_en(pin, 1);  //enable input
	gpio_setup_up_down_resistor(pin, pulup);
}

void gpio_output_init(GPIO_PinTypeDef pin, uint8_t value) {
	gpio_set_func(pin, AS_GPIO); //set pin gpio
    gpio_write(pin, value);
	gpio_set_output_en(pin, 1); //enable output
	gpio_set_input_en(pin, 1);  //enable input
}

extern u32 scan_pins[1]; // in drv_keyboard.c

#if USE_CFG_GPIO

dev_gpios_t  dev_gpios_new;

static void save_fgpio(flash_tab_gpios_t * pftab) {
	pftab->id[0] = tab_gpios.id[0];
	pftab->id[1] = tab_gpios.id[1];
	pftab->crc = xcrc32((uint8_t *)&pftab->gpios, sizeof(tab_gpios.gpios), 0xffffffff);
	flash_erase(FLASH_ADDR_TAB_GPIOS);
	flash_write_page(FLASH_ADDR_TAB_GPIOS, sizeof(flash_tab_gpios_t), (uint8_t *) pftab);
}

void save_config_gpio(void) {
	flash_tab_gpios_t ftab;
	flash_read_page(FLASH_ADDR_TAB_GPIOS, sizeof(ftab), (uint8_t *)&ftab);
	if(memcmp(&ftab.gpios, &dev_gpios_new, sizeof(ftab.gpios))) {
		memcpy(&ftab.gpios,&dev_gpios_new, sizeof(ftab.gpios));
		save_fgpio(&ftab);
	}
}

static void flash_gpios_init(void) {
	flash_tab_gpios_t ftab;
	flash_read_page(FLASH_ADDR_TAB_GPIOS, sizeof(ftab), (uint8_t *)&ftab);
	if(tab_gpios.id[0] == ftab.id[0]
		&& tab_gpios.id[1] == ftab.id[1]
		&& xcrc32((uint8_t *)&ftab.gpios, sizeof(ftab.gpios), 0xffffffff) == ftab.crc) {
		memcpy(&dev_gpios, &ftab.gpios, sizeof(dev_gpios));
	} else {
		memcpy(&dev_gpios, &tab_gpios.gpios, sizeof(dev_gpios));
		memcpy(&ftab.gpios, &tab_gpios.gpios, sizeof(dev_gpios));
		save_fgpio(&ftab);
		factory_reset_start(NULL);
	}
	memcpy(&dev_gpios_new, &dev_gpios, sizeof(dev_gpios));
}
#endif

void dev_gpios_init(void) {
#if USE_CFG_GPIO
	flash_gpios_init();
#else
	memcpy(&dev_gpios, &tab_gpios.gpios, sizeof(dev_gpios));
#endif
	if(!dev_gpios.rl)
		dev_gpios.rl = GPIO_RELAY1;
    gpio_output_init(dev_gpios.rl, RELAY_OFF);
	if(!dev_gpios.led1)
		dev_gpios.led1 = GPIO_LED1;
    gpio_output_init(dev_gpios.led1,
    		(dev_gpios.flg & GPIOS_FLG_LED1_POL)? LED_ON : LED_OFF);
    if(dev_gpios.led2)
    	gpio_output_init(dev_gpios.led2,
    			(dev_gpios.flg & GPIOS_FLG_LED2_POL)? LED_ON : LED_OFF);
#if USE_SWITCH
	if(!dev_gpios.sw1)
		dev_gpios.sw1 = GPIO_SWITCH1;
    gpio_input_init(dev_gpios.sw1, PM_PIN_PULLUP_10K);
#endif
	if(!dev_gpios.key)
		dev_gpios.key = GPIO_BUTTON;
    scan_pins[0] = dev_gpios.key;
    gpio_input_init(dev_gpios.key, PM_PIN_PULLUP_10K);
}

void dev_relay_init(void) {
    check_first_start();
    light_blink_start(1, 100, 100);
}



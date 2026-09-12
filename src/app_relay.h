#ifndef SRC_INCLUDE_APP_RELAY_H_
#define SRC_INCLUDE_APP_RELAY_H_

// bits dev_gpios flags:
#define GPIOS_FLG_LED1_POL	1 // not change! -> see get_led() -> return 0 or 1
#define GPIOS_FLG_LED2_POL	2
#define GPIOS_FLG_KEY_ON	4
#define GPIOS_FLG_BTN_ON	8

typedef struct {
    uint16_t		flg;	// LED1-2 inversion bits, ...
    GPIO_PinTypeDef rl;
    GPIO_PinTypeDef led1;
    GPIO_PinTypeDef led2;
    GPIO_PinTypeDef key;
    GPIO_PinTypeDef sw1;
    GPIO_PinTypeDef swire;
#if USE_BL0937
    GPIO_PinTypeDef sel;
    GPIO_PinTypeDef cf;
    GPIO_PinTypeDef cf1;
#endif
#if USE_BL0942
    GPIO_PinTypeDef rx;
    GPIO_PinTypeDef tx;
#endif
} dev_gpios_t;

extern dev_gpios_t dev_gpios;

//--------- Work Data --------
// bits event_blocking_mask, relay_bits_blocking_events
typedef enum {
#if USE_METERING
	BIT_MAX_VOLTAGE_OFF = 0,// 0x01
	BIT_MIN_VOLTAGE_OFF,	// 0x02
	BIT_MAX_CURRENT_OFF,	// 0x04
	BIT_MAX_TEMP_OFF,		// 0x08
	BIT_MIN_TEMP_OFF,		// 0x10
	BIT_ERR_TS_OFF,			// 0x20
#else
	BIT_MAX_TEMP_OFF,		// 0x01
	BIT_MIN_TEMP_OFF,		// 0x02
	BIT_ERR_TS_OFF,			// 0x04
#endif
} relay_bits_emergency_e;

typedef struct {
#if USE_METERING || USE_SENSOR_MY18B20
#if USE_METERING
	uint16_t tik_max_current; // timer count max current, step 1 sec, =0xFFFF - flag end
#endif
	uint16_t tik_start; // step 1 sec, =0xFFFF - flag end
	uint16_t tik_reload; // step 1 sec, =0xFFFF - flag end
	uint8_t  relay_bits_blocking_events; // Blocking Events, relay_bits_emergency_e, reported
#endif // USE_METERING || USE_SENSOR_MY18B20
	uint8_t  relay_state; // cостояние реле
#if USE_BL0942
	uint8_t first_start; // flag, startup = true
#endif
} event_processing_t;

extern event_processing_t ev_wrk;

void gpio_input_init(GPIO_PinTypeDef pin, GPIO_PullTypeDef pulup);
void gpio_output_init(GPIO_PinTypeDef pin, uint8_t value);
void dev_gpios_init(void);

bool get_relay_status(void);
void set_relay_status(bool status);
#if USE_THERMOSTAT // USE_SENSOR_MY18B20
void set_therm_relay_status(bool status);
#endif

#if USE_CFG_GPIO
dev_gpios_t  dev_gpios_new;
void save_config_gpio(void);
#endif

void dev_relay_init(void);

#endif /* SRC_INCLUDE_APP_RELAY_H_ */

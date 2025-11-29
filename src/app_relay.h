#ifndef SRC_INCLUDE_APP_RELAY_H_
#define SRC_INCLUDE_APP_RELAY_H_

//#define AMT_RELAY   1

typedef struct {
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
extern uint8_t relay_off, relay_state;

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

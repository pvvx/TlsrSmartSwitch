#include "app_main.h"

typedef enum {
	BUTTON_FLAG_WAIT_LONG_OFF =	0xff,
	BUTTON_FLAG_WAIT_TST_OFF =	0xfe
} BUTTON_FLAG_t;

app_button_t app_button[MAX_BUTTON_NUM];

/*******************************************************************
 * @brief	Button Keep Pressed
 */
static void buttonKeepPressed(u8 btNum) {
	sws_printf("buttonKeepPressed(%d)\n", btNum);
    if(btNum == VK_SW1) {
    	factory_reset_start(NULL);
    }
}

/*******************************************************************
 * @brief	Button Single Pressed
 *
static void buttonSinglePressed(u8 btNum) {
    if(btNum == VK_SW1
    	&& !cfg_on_off.key_lock) {
   		cmdOnOff_toggle();
    }
}
*/
/*******************************************************************
 * @brief	MultiKey / Button Pressed
 */
static void buttonPressed(u8 btNum, u8 count) {
    sws_printf("buttonPressed(%d,%d)\n", btNum, count);
    if(btNum == VK_SW1) {
    	if(!cfg_on_off.key_lock) {
    		cmdOnOff_toggle();
    	}
    }
#if USE_SWITCH
    else { // if(btNum != VK_SW1) {
		switchAction(SW_ACTION_ON, count);
	}
#endif
}

/*******************************************************************
 * @brief	MultiKey Button end series (Released)
 */
static void buttonSeriesEnd(u8 btNum, u8 count) {
    sws_printf("buttonSeriesEnd(%d,%d)\n", btNum, count);
#if USE_SWITCH
	if(btNum != VK_SW1) {
		switchAction(SW_ACTION_END, count);
	}
#endif
}

/*******************************************************************
 * @brief	MultiKey / Button Released
 */
static void buttonReleased(u8 btNum, u8 count) {
    sws_printf("buttonReleased(%d,%d)\n", btNum, count);
#if USE_SWITCH
	if(btNum != VK_SW1) {
		switchAction(SW_ACTION_OFF, count);
	}
#endif
}


/*******************************************************************
 * @brief	Task keys/buttons
 */
//__attribute__((optimize("-O2")))
void buttonTask(void) {
	u8 state;
	app_button_t * pbt;
	for(int i = 0; i < MAX_BUTTON_NUM; i++) {
		pbt = &app_button[i];
		if(!pbt->gpio_name)
			continue;
		state = (((gpio_read(pbt->gpio_name)) != 0) == pbt->gpio_on);
		if(state) {
			// button on
			if (!pbt->pressed) { // кнопка была отпущена?
				// event button on
				pbt->event_time = clock_time();
			} else // кнопка была нажата
			// обработка остановлена до ожидания интервала отжатой кнопки на BUTTON_OFF_TIME_MS мс?
			if (pbt->wait_off != BUTTON_FLAG_WAIT_LONG_OFF) {
				// обработка не остановлена, кнопка нажата, проверка на дребезг
				if (clock_time() - pbt->event_time
						> BUTTON_BOUNCE_TIME_MS * CLOCK_16M_SYS_TIMER_CLK_1MS) {
					if (!pbt->key_count_on) { // первое нажатие?
						// первое срабатывание после паузы более BUTTON_OFF_TIME_MS мс
						// pbt->key_count_on = 0;
						pbt->wait_off = 1; // далее вызвать условие запуска buttonPressed()
						// buttonSinglePressed(i);
					}
					// кнопка нажата повторно или удержана
					if (pbt->key_count_on != pbt->wait_off) {
						// кнопка нажата впервые или повторно, счет нажатий
						// если счет переполнен, установить кол-во нажатий для factory_reset
						if (++pbt->key_count_on > BUTTON_FR_COUNTER_MAX)
							pbt->key_count_on = BUTTON_FR_COUNTER_MAX;
						// фиксация нового нажатия от повторного вызова buttonPressed()
						pbt->wait_off = pbt->key_count_on;
						buttonPressed(i, pbt->key_count_on);
					} else {
						// кнопка удержана
						if (clock_time() - pbt->event_time
							> BUTTON_KEEP_TIME_MS * CLOCK_16M_SYS_TIMER_CLK_1MS) {
							// кнопка удержана более 5 секунд
							pbt->key_count_on = 0;
							// отключить последующую обработку до истечения интервала
							// с отжатой кнопкой в течении BUTTON_OFF_TIME_MS
							pbt->wait_off = BUTTON_FLAG_WAIT_LONG_OFF;
							buttonKeepPressed(i);
						}
					}
				}
			} // else - обработка остановлена - кнопка ещё не была отпущена на более 500 мс
		} else {
			// button off
			if (pbt->pressed) { // кнопка была нажата?
				// event button off
				pbt->event_time = clock_time();
			} else
			// кнопка отпущена
			if (pbt->wait_off) {
				// включено ожидание длительного (заключительного) интервала отжатой кнопки
				if (pbt->wait_off >= BUTTON_FLAG_WAIT_TST_OFF) {
				// тест дребезга контакта пройден,
				// ждем длительного интервала отжатой кнопки для завершения серии нажатий
					if(clock_time() - pbt->event_time
						> BUTTON_OFF_TIME_MS * CLOCK_16M_SYS_TIMER_CLK_1MS) {
						// кнопка отжата уже более BUTTON_OFF_TIME_MS мс
						// отключить последующие ожидания интервалов отжатой кнопки
						pbt->wait_off = 0;
						if(pbt->key_count_on) {
							// вызов проуцедуры обработки завершения серии нажатий со счетом нажатий
							buttonSeriesEnd(i, pbt->key_count_on);
							pbt->key_count_on = 0;
						}
					}
				} else
				// кнопка отпущена, проверка на дребезг
				if (clock_time() - pbt->event_time
					> BUTTON_BOUNCE_TIME_MS * CLOCK_16M_SYS_TIMER_CLK_1MS) {
					// кнопка отпущена, тест дребезга пройден
					pbt->wait_off = BUTTON_FLAG_WAIT_TST_OFF;
					buttonReleased(i, pbt->key_count_on);
				}
			}
		}
		pbt->pressed = state;
	}
}

/*******************************************************************
 * @brief	Button Initialize
 */
void buttonInit(void) {
	app_button_t * pbt;

	if(!dev_gpios.key)
		dev_gpios.key = GPIO_BUTTON;
	app_button[0].gpio_name = dev_gpios.key;
    app_button[0].gpio_on = (dev_gpios.flg & GPIOS_FLG_KEY_ON) != 0;
#if USE_SWITCH
	if(dev_gpios.sw1) {
#if USE_SENSOR_MY18B20
		if(dev_gpios.sw1 != dev_gpios.swire) {
			app_button[1].gpio_name = dev_gpios.sw1;
			app_button[1].gpio_on = (dev_gpios.flg & GPIOS_FLG_BTN_ON) != 0;
		}
#else
		app_button[1].gpio_name = dev_gpios.sw1;
		app_button[1].gpio_on = (dev_gpios.flg & GPIOS_FLG_BTN_ON) != 0;
#endif
	}
#endif
    for(int i = 0; i < MAX_BUTTON_NUM; i++) {
		pbt = &app_button[i];
		if(pbt->gpio_name) {
			gpio_input_init(pbt->gpio_name, PM_PIN_PULLUP_10K);
			sleep_us(64);
			pbt->event_time = clock_time();
			pbt->pressed = (((gpio_read(pbt->gpio_name)) != 0) == pbt->gpio_on);
		}
		pbt->wait_off = BUTTON_FLAG_WAIT_LONG_OFF;
	}
}


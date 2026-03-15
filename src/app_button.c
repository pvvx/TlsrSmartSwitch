#include "app_main.h"


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
	u32 tt;
	for(int i = 0; i < MAX_BUTTON_NUM; i++) {
		pbt = &app_button[i];
		if(!pbt->gpio_name)
			continue;
		state = (((gpio_read(pbt->gpio_name)) != 0) == pbt->gpio_on);
		tt = clock_time();
		if(state) {
			// button on
			if (!pbt->pressed) { // кнопка была отпущена?
				// event button on
				pbt->event_time = tt;
			} else // кнопка была нажата
			// обработка остановлена до ожидания интервала отжатой кнопки на BUTTON_OFF_TIME_TICK мс?
			if (pbt->wait_off != BUTTON_FLAG_WAIT_LONG_OFF) {
				// обработка не остановлена, кнопка нажата, проверка на дребезг
				if (tt - pbt->event_time > BUTTON_BOUNCE_TIME_TICK) {
					if (pbt->key_count_on == BUTTON_CNT_NONE) { // первое нажатие?
						// первое срабатывание после паузы более BUTTON_OFF_TIME_TICK мс
						// далее вызвать условие запуска buttonPressed()
						pbt->wait_off = BUTTON_CNT_SINGLE_PRESS;
					}
					// кнопка нажата повторно или удержана
					if (pbt->key_count_on != BUTTON_CNT_LONG_PRESS
						&& pbt->key_count_on != pbt->wait_off) {
						// кнопка нажата впервые или повторно, счет нажатий
						// если счет переполнен, установить кол-во нажатий для factory_reset
						if (++pbt->key_count_on > BUTTON_CNT_FACTORY_RESET)
							pbt->key_count_on = BUTTON_CNT_FACTORY_RESET;
						// фиксация нового нажатия от повторного вызова buttonPressed()
						pbt->wait_off = pbt->key_count_on;
						buttonPressed(i, pbt->key_count_on);
					} else {
						// кнопка удержана
						if (tt - pbt->event_time > BUTTON_KEEP_TIME_TICK) {
							// кнопка удержана более 5 секунд
							// pbt->key_count_on = BUTTON_CNT_LONG_PRESS; // уже = 0xff
							// отключить последующую обработку до истечения интервала
							// с отжатой кнопкой в течении BUTTON_OFF_TIME_TICK
							pbt->wait_off = BUTTON_FLAG_WAIT_LONG_OFF;
							buttonKeepPressed(i);
						} else if(pbt->key_count_on != BUTTON_CNT_LONG_PRESS
							// небыло срабатывания по долгому нажатию
							&& tt - pbt->event_time > BUTTON_LONG_PRESS_TIME_TICK) {
							// кнопка удержана более 750 ms
							pbt->key_count_on = BUTTON_CNT_LONG_PRESS; // долгое нажатие
							buttonPressed(i, pbt->key_count_on);
						}
					}
				}
			} // else - обработка остановлена - кнопка ещё не была отпущена на более 500 мс
		} else {
			// button off
			if (pbt->pressed) { // кнопка была нажата?
				// event button off
				pbt->event_time = tt;
			} else
			// кнопка отпущена
			if (pbt->wait_off) {
				// включено ожидание длительного (заключительного) интервала отжатой кнопки
				if (pbt->wait_off >= BUTTON_FLAG_WAIT_TST_OFF) {
				// тест дребезга контакта пройден,
				// ждем длительного интервала отжатой кнопки для завершения серии нажатий
					if(tt - pbt->event_time	> BUTTON_OFF_TIME_TICK) {
						// кнопка отжата уже более BUTTON_OFF_TIME_TICK мс
						// отключить последующие ожидания интервалов отжатой кнопки
						pbt->wait_off = BUTTON_FLAG_OFF;
						if(pbt->key_count_on) {
							// вызов проуцедуры обработки завершения серии нажатий со счетом нажатий
							buttonSeriesEnd(i, pbt->key_count_on);
							pbt->key_count_on = BUTTON_CNT_NONE;
						}
					}
				} else
				// кнопка отпущена, проверка на дребезг
				if (tt - pbt->event_time > BUTTON_BOUNCE_TIME_TICK) {
					// кнопка отпущена, тест дребезга пройден
					pbt->wait_off = BUTTON_FLAG_WAIT_TST_OFF;
					// тут возможен вызов с pbt->key_count_on = 0, если небыл пройден дребезг нажатия
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
// специальные установки GPIO
// для данного варианта прошивки (из спец.таблицы)
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
// типовой buttonInit()
    for(int i = 0; i < MAX_BUTTON_NUM; i++) {
		pbt = &app_button[i];
		if(pbt->gpio_name) {
			gpio_input_init(pbt->gpio_name, PM_PIN_PULLUP_10K);
			sleep_us(32);
			pbt->event_time = clock_time();
			pbt->pressed = (((gpio_read(pbt->gpio_name)) != 0) == pbt->gpio_on);
		}
		pbt->wait_off = BUTTON_FLAG_WAIT_LONG_OFF;
	}
}


#ifndef SRC_INCLUDE_APP_BUTTON_H_
#define SRC_INCLUDE_APP_BUTTON_H_

#define MAX_BUTTON_MULTI_PRESSED    5

#ifndef MAX_BUTTON_NUM
#if USE_SWITCH
#define MAX_BUTTON_NUM 2
#else
#define MAX_BUTTON_NUM 1
#endif
#endif

#define BUTTON_OFF_TIME_MS	500
#define BUTTON_BOUNCE_TIME_MS 10
#define BUTTON_KEEP_TIME_MS	5000

#define BUTTON_FR_COUNTER_MAX 10      /* number for factory reset */

typedef struct {
	u32 event_time; // фиксация времени события нажатия и отпускания кнопки
	u16 gpio_name; // номер GPIO кнопки
	u8 gpio_on; // уровень на выводе при нажатой кнопке (0 или 1)
	u8 pressed; // текущее состояние кнопки
	u8 key_count_on; // счет нажатий кнопки с паузами до BUTTON_OFF_TIME_MS мс
	u8 wait_off; // флаг остановки обработки до ожидания отпускания кнопки и счет ожиданий отпускания кнопки
} app_button_t;

extern app_button_t app_button[MAX_BUTTON_NUM];

void buttonInit(void);
void buttonTask(void);

#endif /* SRC_INCLUDE_APP_BUTTON_H_ */

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

#define BUTTON_OFF_TIME_TICK		(500*CLOCK_16M_SYS_TIMER_CLK_1MS) // интервал отпущенной кнопки для события конец серии нажатий
#define BUTTON_BOUNCE_TIME_TICK 	(10*CLOCK_16M_SYS_TIMER_CLK_1MS)  // интервал подавления дребезга
#define BUTTON_LONG_PRESS_TIME_TICK	(750*CLOCK_16M_SYS_TIMER_CLK_1MS) // интервал до события длительного нажатия кнопки
#define BUTTON_KEEP_TIME_TICK		(5000*CLOCK_16M_SYS_TIMER_CLK_1MS) // интервал удержания кнопки до события сброса устройства

#define BUTTON_FR_COUNTER_MAX 10      /* number for factory reset */

typedef enum {
	BUTTON_CNT_NONE =	0,
	BUTTON_CNT_SINGLE_PRESS, // = ACTION_SINGLE
	BUTTON_CNT_DOUBLE_PRESS, // = ACTION_DOUBLE
	BUTTON_CNT_TRIPLE_PRESS, // = ACTION_TRIPLE
	/* BUTTON_CNT_4 ... BUTTON_CNT_9 = ACTION_N */
	BUTTON_CNT_FACTORY_RESET = 10,
	BUTTON_CNT_LONG_PRESS =	0xff // = ACTION_HOLD
} BUTTON_CNT_t;

typedef enum {
	BUTTON_FLAG_OFF = 0,
	BUTTON_FLAG_WAIT_TST_OFF = 0xfe,
	BUTTON_FLAG_WAIT_LONG_OFF =	0xff
} BUTTON_FLAG_t;

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

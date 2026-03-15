#ifndef SRC_INCLUDE_APP_SWITCH_H_
#define SRC_INCLUDE_APP_SWITCH_H_

typedef enum {
    ACTION_RELEASE = 0, // Отпускание после длинного нажатия.
    ACTION_SINGLE,
    ACTION_DOUBLE,
    ACTION_TRIPLE,
    ACTION_QUADRUPLE,
    ACTION_QUINTUPLE,
    ACTION_HOLD = 255 // Начало длинного нажатия.
} switch_zb_action_t;

typedef enum {
    SW_ACTION_OFF = 0,	// = ZCL_ONOFF_STATUS_OFF
	SW_ACTION_ON,		// = ZCL_ONOFF_STATUS_ON
	SW_ACTION_END
} switch_action_t;


#if USE_SWITCH
void switchAction(switch_action_t action, u8 count);
void switchFirstStart(void);
#endif

#endif /* SRC_INCLUDE_APP_SWITCH_H_ */

#ifndef _MZSW01_BL0937_MY18B20_H_
#define _MZSW01_BL0937_MY18B20_H_

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif

/**********************************************************************
 * Product Information
 * max 24 symbols
 */
#define ZCL_BASIC_MFG_NAME     {11,'T','e','l','i','n','k','-','p','v','v','x'}
#define ZCL_BASIC_MODEL_ID     {15,'M','Z','S','W','0','2','_','B','L','0','9','3','7','_','z'}

#define USE_SENSOR_MY18B20		1

/************************* Configure MY18B20 GPIO ***************************************/
#if USE_SENSOR_MY18B20
#define GPIO_ONEWIRE1           GPIO_PA1
#endif
/************************* Configure SWITCH GPIO ***************************************/
#if USE_SWITCH
#define SWITCH1_GPIO            GPIO_PA1
#endif

#define PA1_DATA_OUT            0
#define PA1_INPUT_ENABLE        ON
#define PA1_OUTPUT_ENABLE       OFF
#define PA1_FUNC                AS_GPIO
#define PULL_WAKEUP_SRC_PA1     PM_PIN_PULLUP_10K

/************************* Configure KEY GPIO ***************************************/
#define MAX_BUTTON_NUM  1

#define BUTTON                  GPIO_PD7
#define PD7_INPUT_ENABLE        ON
#define PD7_DATA_OUT            OFF
#define PD7_OUTPUT_ENABLE       OFF
#define PD7_FUNC                AS_GPIO
#define PULL_WAKEUP_SRC_PD7     PM_PIN_PULLUP_10K

enum {
    VK_SW1 = 0x01,
};

#define KB_MAP_NORMAL   {\
        {VK_SW1,}}

#define KB_MAP_NUM      KB_MAP_NORMAL
#define KB_MAP_FN       KB_MAP_NORMAL

#define KB_DRIVE_PINS  {NULL }
#define KB_SCAN_PINS   {BUTTON}

/************************** Configure LED ****************************************/

#define LED_ON                  0
#define LED_OFF                 1

#define LED_GPIO                GPIO_PB1
#define PB1_FUNC                AS_GPIO
#define PB1_OUTPUT_ENABLE       ON
#define PB1_INPUT_ENABLE        OFF


/********************* Configure Relay ***************************/

#define RELAY_ON                1
#define RELAY_OFF               0

#define RELAY1_GPIO             GPIO_PD2
#define PD2_FUNC                AS_GPIO
#define PD2_OUTPUT_ENABLE       ON
#define PD2_INPUT_ENABLE        ON
#define PD2_DATA_OUT            RELAY_OFF

/**************************** Configure pins for BL0937 ***********************************/

#define USE_BL0937				1
#define GPIO_CF             	GPIO_PB4
#define GPIO_CF1             	GPIO_PB5
#define GPIO_SEL             	GPIO_PD3

#define PB4_FUNC                AS_GPIO
#define PB4_INPUT_ENABLE        ON

#define PC3_FUNC                AS_GPIO
#define PC3_INPUT_ENABLE        ON

#define PD3_FUNC                AS_GPIO
#define PD3_INPUT_ENABLE        ON
#define PD3_OUTPUT_ENABLE       ON
#define PD3_DATA_OUT 			0

#define MAX_VOLTAGE_DEF			26000 // 260.00V
#define MIN_VOLTAGE_DEF			18000 // 180.00V
#define MAX_CURRENT_DEF			25000 // 25.000A
#define PERIOD_MAX_CURRENT_DEF	8	// 8 sec
#define PERIOD_RELOAD_DEF		0	// sec
#define PERIOD_START_DEF		0	// sec

/********************* Configure Vbat ***************************/

#define USE_BATTERY			BATTERY_DC_DC

#define SHL_ADC_VBAT        1  // "B0P" in adc.h
#define GPIO_VBAT           GPIO_PB0 // missing pin on case TLSR8251F512ET24
#define PB0_INPUT_ENABLE    1
#define PB0_DATA_OUT        1
#define PB0_OUTPUT_ENABLE   1
#define PB0_FUNC            AS_GPIO


/********************* Configure printf UART ***************************/

#if UART_PRINTF_MODE
#define DEBUG_INFO_TX_PIN       UART_TX_PD3    //printf
#define DEBUG_BAUDRATE          115200

#endif /* UART_PRINTF_MODE */

#endif /* _MZSW01_BL0937_MY18B20_H_ */


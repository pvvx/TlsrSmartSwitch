/**********************************************************************
 * @file    app_cfg.h
 *
 * @brief   This is the header file for app_cfg
 *********************************************************************/
#ifndef _APP_CFG_H_
#define _APP_CFG_H_

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif

#include "app_types.h"

#define MCU_CORE_8258       1
#define ZB_ROUTER_ROLE      1

/**********************************************************************
 * Version configuration
 */

#include "version_cfg.h"

/**********************************************************************
 * Product Information
 */

#define ZB_TX_POWER_IDX_DEF RF_POWER_INDEX_P10p46dBm

/* Debug mode config */
#define UART_PRINTF_MODE                OFF//ON
#define DEBUG_SAVE	                    ON
#define DEBUG_BUTTON                    ON
#define DEBUG_SWITCH                    ON
#define DEBUG_MONITORING                OFF
#define DEBUG_ONOFF                     ON
#define DEBUG_TIME                      OFF
#define DEBUG_REPORTING                 OFF
#define DEBUG_OTA                       OFF
#define DEBUG_STA_STATUS                OFF

#define USB_PRINTF_MODE                 OFF

/* PM */
#define PM_ENABLE                       OFF

/* PA */
#define PA_ENABLE                       OFF


#define CLOCK_SYS_CLOCK_HZ          48000000 // 32000000    // 24000000 //48000000

/* FLASH 1M map:
              1M
0x100000  ------------
         |  MAC_Addr  |
 0xFF000 |------------|
         | F_CFG_Info | // FACTORY_CFG_BASE_ADD
 0xFE000 |------------|
         | U_Cfg_Info | // 0xFC000 CFG_FACTORY_RST_CNT
 0xFC000 |------------|
         | USER_DATA  |
 0x96000 |------------|
         |     NV     |
 0x80000 |------------|
         |            |
         |            |
         |  OTA_Image | 256K
         |            |
         |            |
 0x40000 |------------|
         |            | // 0x32000 FLASH_ADDR_TAB_GPIOS
         |            |
         |  Firmware  | 256K
         |            |
         |            |
 0x00000  ------------
*/
#define BEGIN_USER_DATA_F1M            0x96000   // begin address for saving energy
#define END_USER_DATA_F1M              0xFC000   // end address for saving energy
/*  Flash 512k map:
     0x80000  ------------
             |            |
             |    NV_2    |
             |            |
     0x7A000 |------------|
             | U_Cfg_Info | // 0x79000 CFG_FACTORY_RST_CNT
     0x78000 |------------|
             | F_CFG_Info | // 0x77000 FACTORY_CFG_BASE_ADD
     0x77000 |------------|
             |  MAC_Addr  |
     0x76000 |------------|
             | USER_DATA  |
     0x72000 |------------|
             |            |
             |  OTA_Image | 200k
             |            |
     0x40000 |------------|
             |            |
             |    NV_1    |
             |            |
     0x34000 |------------|
             |   free     | // 0x32000 FLASH_ADDR_TAB_GPIOS
     0x32000 |------------|
             |            |
             |  Firmware  | 200k
             |            |
     0x00000  ------------
 */
#define BEGIN_USER_DATA_F512K             0x72000 // begin address for saving energy
#define END_USER_DATA_F512K               0x76000 // end address for saving energy
#define FLASH_ADDR_TAB_GPIOS		  	  0x32000
/** Store zigbee information in flash:
 ********************************************************************************************************
 *   Module ID                  |          512K Flash               |              1M Flash             |
 * -----------------------------|-----------------------------------|-----------------------------------|
 * NV_MODULE_ZB_INFO            |        0x34000 - 0x36000          |	      0x80000 - 0x82000         |
 * NV_MODULE_ADDRESS_TABLE      |        0x36000 - 0x38000          |         0x82000 - 0x84000         |
 * NV_MODULE_APS                |        0x38000 - 0x3a000          |         0x84000 - 0x86000         |
 * NV_MODULE_ZCL                |        0x3a000 - 0x3c000          |         0x86000 - 0x88000         |
 * NV_MODULE_NWK_FRAME_COUNT	|        0x3c000 - 0x3e000          |         0x88000 - 0x8a000         |
 * NV_MODULE_OTA                |        0x3e000 - 0x40000          |         0x8a000 - 0x8c000         |
 * NV_MODULE_APP                |        0x7a000 - 0x7c000          |         0x8c000 - 0x8e000         |
 * NV_MODULE_KEYPAIR            |        0x7c000 - 0x80000          |         0x8e000 - 0x96000         |
 *                              |    *16K - can store 127 nodes     |     *32K - can store 302 nodes    |
 * NV_MAX_MODULS
 */
#ifndef BOARD // in "version_cfg.h"
#error "Define BOARD!"
#endif

#define USE_CFG_GPIO	1

#define ZCL_BASIC_MFG_NAME     {11,'T','e','l','i','n','k','-','p','v','v','x'}

#if USE_BL0937
#define _MODEL_EM	'8'
#else
 #if USE_BL0942
	#define _MODEL_EM	'1'
 #else
	#error "Config error!"
 #endif
#endif

#if USE_SWITCH
#define _MODEL_SW	'2'
#else
#define _MODEL_SW	'1'
#endif

#if USE_SENSOR_MY18B20
 #if USE_THERMOSTAT
	#define ZCL_BASIC_MODEL_ID     {10,'E','M',_MODEL_EM,'S','W',_MODEL_SW,'T','S','_','z'}
 #else
	#define ZCL_BASIC_MODEL_ID     {9,'E','M',_MODEL_EM,'S','W',_MODEL_SW,'T','_','z'}
 #endif
#else
#define ZCL_BASIC_MODEL_ID     {8,'E','M',_MODEL_EM,'S','W',_MODEL_SW,'_','z'}
#endif

/*** Configure GPIOS for my device ***/

#ifdef MY_DEVICE

#if USE_BL0937

#define BUTTON_ON		0
#define GPIO_BUTTON     GPIO_PD7

#define LED_ON          0
#define GPIO_LED1       GPIO_PB1
#define GPIO_LED2       GPIO_PB2

#define RELAY_ON        1
#define GPIO_RELAY1     GPIO_PD2

#define GPIO_SWITCH_ON  0
#define GPIO_SWITCH1    GPIO_PA1

#define GPIO_ONEWIRE1   GPIO_PA1

#define GPIO_CF         GPIO_PB4
#define GPIO_CF1        GPIO_PB5
#define GPIO_SEL        GPIO_PD3

#define BL0937_CURRENT_REF        (191547) 		// x1000: 0..65.535A (divisor = 1000 - > A)
#define BL0937_VOLTAGE_REF        (208773)  	// x100: 0..655.35V (divisor = 100 - > V)
#define BL0937_POWER_REF          (1161624) 	// x100 0..327.67W, x10: 327.67..3276.7W (divisor = 10, 100 - > W)
#define BL0937_ENERGY_REF         ((BL0937_POWER_REF + 225)/450) //(=2403) x100 Wh (divisor = 100000 - > kWh)

#endif // USE_BL0937

#if	USE_BL0942

#define BUTTON_ON		0
#define GPIO_BUTTON     GPIO_PB5

#define LED_ON          0
#define GPIO_LED1       GPIO_PB4
#define GPIO_LED2       0

#define RELAY_ON        1
#define GPIO_RELAY1     GPIO_PC3

#define GPIO_SWITCH_ON 	0
#define GPIO_SWITCH1    GPIO_PD2

#define GPIO_ONEWIRE1   GPIO_PD2

#define GPIO_UART_TX    UART_TX_PB1
#define GPIO_UART_RX    UART_RX_PB7

#define BL0942_CURRENT_REF      16860520 // 2pow32/251.21346469622 // x1000: 0..65.535A
#define BL0942_VOLTAGE_REF      26927694 // 2pow32/159.5 // x100: 0..655.35V
// POWER_REF = (2pow32/VOLTAGE_REF)*(2pow32/CURRENT_REF)*353700/305978/73989 = 0.63478593422
#define BL0942_POWER_REF        27060025 // 2pow24/0.620  // x1000: x1000: 0..327.67W, x100 32.767..327.67W, x10: 327.67..3276.7W
// ENERGY_REF = ((2pow24/POWER_REF)*36000)/419430.4 = 0.053215
#define BL0942_ENERGY_REF       315272310 // 2pow24/0.053215 // x100000
#define BL0942_FREQ_REF         100000000 // (measured: 100175000) x100

#endif // USE_BL0942

#else // MY_DEVICE

/*** Configure GPIOS for device BL0937 ***/

#if	USE_BL0937

#define BUTTON_ON		0
#define GPIO_BUTTON     GPIO_PD7

#define LED_ON          0
#define GPIO_LED1       GPIO_PB1
#define GPIO_LED2       0

#define RELAY_ON        1
#define GPIO_RELAY1     GPIO_PD2

#define GPIO_SWITCH_ON  0
#define GPIO_SWITCH1    GPIO_PA1

#define GPIO_ONEWIRE1   GPIO_PA1

#define GPIO_CF         GPIO_PB4
#define GPIO_CF1        GPIO_PB5
#define GPIO_SEL        GPIO_PD3

#endif

/*** Configure GPIOS for device BL0942 ***/

#if	USE_BL0942

#define BUTTON_ON		0
#define GPIO_BUTTON     GPIO_PB5

#define LED_ON          0
#define GPIO_LED1       GPIO_PB4
#define GPIO_LED2       0

#define RELAY_ON        1
#define GPIO_RELAY1     GPIO_PC3

#define GPIO_SWITCH_ON 	0
#define GPIO_SWITCH1    GPIO_PD2

#define GPIO_ONEWIRE1   GPIO_PD2

#define GPIO_UART_TX    UART_TX_PB1
#define GPIO_UART_RX    UART_RX_PB7

#endif

#endif // MY_DEVICE

#define KB_LINE_HIGH_VALID BUTTON_ON
#define BUTTON_OFF 		(!BUTTON_ON)
#define LED_OFF         (!LED_ON)
#define RELAY_OFF   	(!RELAY_ON)
#define GPIO_SWITCH_OFF	(!GPIO_SWITCH_ON)


/*** Configure printf UART ***/

#if UART_PRINTF_MODE
#define DEBUG_INFO_TX_PIN       UART_TX_PD3    //printf
#define DEBUG_BAUDRATE          115200

#endif /* UART_PRINTF_MODE */

/*** Configure button ***/

enum {
    VK_SW1 = 0x01,
};

#define MAX_BUTTON_NUM  1

#define KB_MAP_NORMAL   {{VK_SW1,}}

#define KB_MAP_NUM      KB_MAP_NORMAL
#define KB_MAP_FN       KB_MAP_NORMAL

#define KB_DRIVE_PINS  {NULL}
#define KB_SCAN_PINS   {GPIO_BUTTON}

/**********************************************************************
 * Battery & RF Power
 */

/*** Configure  GPIO Vbat ***/

#define SHL_ADC_VBAT        1  // "B0P" in adc.h
#define GPIO_VBAT           GPIO_PB0 // missing pin on case TLSR8251F512ET24
#define PB0_INPUT_ENABLE    1
#define PB0_DATA_OUT        1
#define PB0_OUTPUT_ENABLE   1
#define PB0_FUNC            AS_GPIO
#define PULL_WAKEUP_SRC_PB0 PM_PIN_PULLUP_10K

/* Voltage detect module */
#define VOLTAGE_DETECT_ENABLE       0 // always = 0!
#define VOLTAGE_DETECT_ADC_PIN      GPIO_VBAT

/**********************************************************************
 * NVM configuration
 */

// ID compatible NV, and flag used NV_MODULE_APP
#define USE_NV_APP  ((APP_RELEASE << 24) | (APP_BUILD << 16) | (0x1000 | (CHIP_TYPE << 8) | BOARD))    // ID, not change!
#define USE_NV_APP_OK  ((0 << 24) | (0x02 << 16) | (0x1000 | (CHIP_TYPE << 8) | BOARD))  // Test for compatible version of saved settings formats

typedef enum{
    NV_ITEM_APP_DEV_VER = 0x60, // see sdk/proj/drivers/drv_nv.h
    NV_ITEM_APP_DEV_NAME,
    NV_ITEM_APP_MAN_NAME,
    NV_ITEM_APP_THERMOSTAT_UI_CFG,
    NV_ITEM_APP_PIR_CFG,
    NV_ITEM_APP_TRIGGER_UI_CFG,
	NV_ITEM_APP_CFG_RELAY,
	NV_ITEM_APP_CFG_MIN_MAX,
	NV_ITEM_APP_CFG_SENSOR_BL09xx,
	NV_ITEM_APP_CFG_SENSOR_MY18B20,
	NV_ITEM_APP_CFG_THERMOSTAT,
	NV_ITEM_APP_CFG_ON_OFF,
} nv_item_app_t;

/**********************************************************************
 * ZCL cluster support setting
 */

/* BDB */
#define TOUCHLINK_SUPPORT               ON
#define FIND_AND_BIND_SUPPORT           OFF

#define ZCL_ON_OFF_SUPPORT              ON
#define ZCL_LEVEL_CTRL_SUPPORT          OFF // =0 (!)
#define ZCL_LIGHT_COLOR_CONTROL_SUPPORT OFF // =0 (!)

//#define ZCL_POLL_CTRL_SUPPORT                      1 ?
#define ZCL_POWER_CFG_SUPPORT                       OFF
#define ZCL_GROUP_SUPPORT                           ON
#define ZCL_SCENE_SUPPORT                           ON
#define ZCL_ON_OFF_SWITCH_CFG_SUPPORT               USE_SWITCH
#define ZCL_OTA_SUPPORT                             ON
#define ZCL_GP_SUPPORT                              ON
#define ZCL_METERING_SUPPORT                        ON
#define ZCL_ELECTRICAL_MEASUREMENT_SUPPORT          ON
#define ZCL_MULTISTATE_INPUT_SUPPORT                USE_SWITCH
#define ZCL_THERMOSTAT_SUPPORT						USE_THERMOSTAT
#define ZCL_TEMPERATURE_MEASUREMENT_SUPPORT			USE_SENSOR_MY18B20

// TODO: ZCL_ALARMS_SUPPORT

#if TOUCHLINK_SUPPORT
#define ZCL_ZLL_COMMISSIONING_SUPPORT               ON
#endif

/**********************************************************************
 * Modules configuration
 */

//#define ZB_DEFAULT_TX_POWER_IDX ZB_TX_POWER_IDX_DEF

#define ON                  1
#define OFF                 0

/* for reporting */
#define REPORTING_MIN       60              /* 1 min            */
#define REPORTING_MAX       300             /* 5 min            */

/* Watch dog module */
#define MODULE_WATCHDOG_ENABLE                      0

#if (ZBHCI_USB_PRINT || ZBHCI_USB_CDC || ZBHCI_USB_HID || ZBHCI_UART)
    #define ZBHCI_EN                                1
#endif

/**********************************************************************
 * Stack configuration
 */
#include "stack_cfg.h"

/**********************************************************************
 * EV configuration
 */
typedef enum{
    EV_POLL_ED_DETECT,
    EV_POLL_HCI,
    EV_POLL_IDLE,
    EV_POLL_MAX,
}ev_poll_e;

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif

#endif // _APP_CFG_H_

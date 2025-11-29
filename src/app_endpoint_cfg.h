#ifndef SRC_INCLUDE_APP_ENDPOINT_CFG_H_
#define SRC_INCLUDE_APP_ENDPOINT_CFG_H_

#define APP_ENDPOINT1 0x01
#define APP_ENDPOINT2 0x02
#define APP_ENDPOINT3 0x03


/* Custom Attr for Relay (OnOff cluster) */
#define ZCL_ATTRID_RELAY_STATE				0xF000 // RR, On/Off

/* Custom Attr for OnOff cluster */
#define ZCL_ATTRID_CUSTOM_KEY_LOCK          0xF001
#define ZCL_ATTRID_CUSTOM_LED               0xF002

/* Custom Attr for OnOff config cluster */
#define CUSTOM_ATTRID_SWITCH_TYPE           0xF003
#define CUSTOM_ATTRID_DECOUPLED             0xF004

/* Custom Attr for Electrical Measurement cluster */
#define ZCL_ATTRID_EMERGENCY_OFF			0xF005 // RW
#define ZCL_ATTRID_ALARM_FLAGS				0xF006 // RWR, 8 bits_emergency_off_t
#define ZCL_ATTRID_CURRENT_COEF       		0xF007
#define ZCL_ATTRID_VOLTAGE_COEF        		0xF008
#define ZCL_ATTRID_POWER_COEF         		0xF009
#define ZCL_ATTRID_ENERGY_COEF         		0xF00A
#define ZCL_ATTRID_FGREQ_COEF         		0xF00B

/* Custom Attr (if USE_SENSOR_MY18B20) for TemperatureMeasurement */
#define ZCL_TEMPERATURE_SENSOR_ID			0xF00C // R, uint32 id MY18B20
#define ZCL_TEMPERATURE_SENSOR_ERRORS		0xF00D // R, 8 bits_emergency_off_t
#define ZCL_TEMPERATURE_SENSOR_MULTIPLER	0xF00E
#define ZCL_TEMPERATURE_SENSOR_ZERO			0xF00F
#define ZCL_TEMPERATURE_SENSOR_HYSTERESIS	0xF010 // uses thermostat
#define ZCL_TEMPERATURE_MIN					0xF011 // emergency
#define ZCL_TEMPERATURE_MAX					0xF012 // emergency

#if USE_CFG_GPIO
#define ZCL_ATTRID_GPIO_RELAY				0xF100
#define ZCL_ATTRID_GPIO_LED1				0xF101
#define ZCL_ATTRID_GPIO_LED2				0xF102
#define ZCL_ATTRID_GPIO_KEY					0xF103
#define ZCL_ATTRID_GPIO_SW1					0xF104
#if USE_SENSOR_MY18B20
#define ZCL_ATTRID_GPIO_SWIRE				0xF105
#endif
#if USE_BL0937
#define ZCL_ATTRID_GPIO_SEL					0xF106
#define ZCL_ATTRID_GPIO_CF					0xF107
#define ZCL_ATTRID_GPIO_CF1					0xF108
#endif
#if USE_BL0942
#define ZCL_ATTRID_GPIO_RX					0xF109
#define ZCL_ATTRID_GPIO_TX					0xF10A
#endif
#endif

/**
 *  @brief Defined for basic cluster attributes
 */
typedef struct {
    uint8_t  zclVersion;
    uint8_t  appVersion;
    uint8_t  stackVersion;
    uint8_t  hwVersion;
    uint8_t  manuName[ZCL_BASIC_MAX_LENGTH];
    uint8_t  modelId[ZCL_BASIC_MAX_LENGTH];
    uint8_t  dateCode[ZCL_BASIC_MAX_LENGTH];
    uint8_t  powerSource;
    uint8_t  genDevClass;                        //attr 8
    uint8_t  genDevType;                         //attr 9
    uint8_t  deviceEnable;
    uint8_t  swBuildId[ZCL_BASIC_MAX_LENGTH];    //attr 4000
} zcl_basicAttr_t;


enum{
	TH_SMODE_OFF = 0,
	TH_SMODE_AUTO,  // 1
	TH_SMODE_NONE,	// 2
	TH_SMODE_COOL,	// 3
	TH_SMODE_HEAT,	// 4
	TH_SMODE_EHEAT,	// 5
	TH_SMODE_PRECOOL,// 6
	TH_SMODE_FAN,	// 7
	TH_SMODE_DRY,	// 8
	TH_SMODE_SLEEP,	// 9
};

typedef struct {
	int16_t temp_cooling; // in 0.01 C
	int16_t temp_heating; // in 0.01 C
	/* sys_mode:
	0 - Off
	1 - Auto
	3 - Cool
	4 - Heat
	5 - Emergency heating
	6 - Precooling
	7 - Fan only
	8 - Dry
	9 - Sleep */
	uint8_t sys_mode;
	int8_t temp_z8;
} zcl_thermostatAttr_save_t;

typedef struct {
	zcl_thermostatAttr_save_t cfg;
//	uint32_t errors; // AC_ERROR_CODE?
	int16_t local_temp; // in 0.01 C
	int16_t min_temp; // in 0.01 C
	int16_t max_temp; // in 0.01 C
	/* relay_state:
	 * bit0: Heat State On,
	 * bit1: Cool State On,
	 * bit2: Fan State On */
	uint16_t relay_state;
	uint8_t cool_on; // 0..100%
	uint8_t healt_on; // 0..100%
	/* operation:
	0 - Cooling Only
	1 - Cooling With Reheat
	2 - Heating Only
	3 - Heating With Reheat
	4 - Cooling and Heating 4-pipes
	5 - Cooling and Heating 4-pipes with Reheat */
	uint8_t operation; // = 5
	/* run_mode:
	0 - Off
	3 - Cool
	4 - Heat */
	uint8_t run_mode;
//	int32_t summ_temp;
//	uint8_t summ_cnt;
	//
	uint8_t occupancy; // = 0  unoccupied, = 1 occupied
//	uint8_t remote_sensing; // = 0
} zcl_thermostatAttr_t;

extern zcl_thermostatAttr_t zcl_thermostat_attrs;

/**
 *  @brief Defined for identify cluster attributes
 */
typedef struct{
    uint16_t identifyTime;
}zcl_identifyAttr_t;

/**
 *  @brief Defined for group cluster attributes
 */
typedef struct{
    uint8_t  nameSupport;
}zcl_groupAttr_t;

/**
 *  @brief Defined for scene cluster attributes
 */
typedef struct{
    uint8_t   sceneCount;
    uint8_t   currentScene;
    uint8_t   nameSupport;
    bool sceneValid;
    uint16_t  currentGroup;
}zcl_sceneAttr_t;

typedef struct {
    uint32_t time_utc;
    uint32_t time_local;
    uint8_t  time_status;
} zcl_timeAttr_t;

/**
 *  @brief Defined for on/off cluster attributes
 */

typedef enum {
    CONTROL_LED_OFF = 0,
    CONTROL_LED_ON,
    CONTROL_LED_ON_OFF
} control_led_t;

typedef struct {
	bool     onOff;
	uint8_t  startUpOnOff;
	uint8_t  key_lock;
	uint8_t  led_control;
    uint8_t  switchType;         // 0x00 - toggle, 0x01 - momentary, 0x02 - multifunction, 0x03 - thermostat
#if USE_SWITCH
    uint8_t  switchActions;
    uint8_t  switchDecoupled;
#endif
} config_on_off_t; // save

extern config_on_off_t cfg_on_off, cfg_on_off_saved;

typedef struct {
//    uint16_t onTime;
//    uint16_t offWaitTime;
    bool     globalSceneControl;
} zcl_onOffAttr_t;


/**
 *  @brief Defined for multistate input clusters attributes
 */
typedef struct {
    uint8_t		out_of_service;
    uint16_t	value;
    uint16_t    num;
    uint8_t     status_flag;
} zcl_msInputAttr_t;

typedef struct {
    uint64_t cur_sum_delivered;    // in 0.01 Wh
    uint32_t mutipler;				// = 1
    uint32_t divisor;				// = 100000
    uint8_t  unit_of_measure;       // 0x00 - kWh
    uint8_t  status;				// = 0
    uint8_t  summation_formatting;  // Bits 0 to 2: Number of Digits to the right of the Decimal Point
                                    // Bits 3 to 6: Number of Digits to the left of the Decimal Point
                                    // Bit  7:      If set, suppress leading zeros
    uint8_t  device_type;			// = 0

} zcl_seAttr_t;

typedef struct {
    uint32_t type;
#if USE_BL0942
    uint16_t freq; // in 0.01 Hz (mutipler/divisor)
#endif
    uint16_t current; // in 0.001 A (mutipler/current_divisor)
    uint16_t voltage; // in 0.01 V (mutipler/divisor)
    uint16_t power; // power div 10,100,1000: in 0.1, 0.01, 0.001 W (mutipler/power_divisor)
    uint16_t mutipler; // mutipler for all
    uint16_t divisor; // voltage and freq div 100
    uint16_t power_divisor; // power div 10,100,1000: in 0.1, 0.01, 0.001 W
    uint16_t current_divisor; // current div 1000, in 0.001A
} zcl_msAttr_t;

// bits emergency_off
typedef enum {
	BIT_MAX_VOLTAGE_OFF = 0,// 0x01
	BIT_MIN_VOLTAGE_OFF,	// 0x02
	BIT_MAX_CURRENT_OFF,	// 0x04
	BIT_MAX_TEMP_OFF,		// 0x08
	BIT_MIN_TEMP_OFF,		// 0x10
} bits_emergency_off_t;

typedef struct {
    int16_t max_voltage; // in 0.01V, = 0 - off
    int16_t min_voltage; // in 0.01V, = 0 - off
    int16_t max_current; // in 0.001A, = 0 - off
    uint16_t time_max_current; // in sec, minimum 8, step 8, = 0 - off
    uint16_t time_reload; // in sec, minimum 8, step 8, = 0 - off
    uint16_t time_start; // in sec, minimum 8, step 8, = 0 - off
    uint8_t emergency_off; // emergency_off_t
} zcl_config_min_max_t;

extern zcl_config_min_max_t config_min_max;

/**
 *  @brief Defined for temperature cluster attributes
 */
typedef struct {
	s16 measuredValue;
	s16 minValue;
	s16 maxValue;
	u16 tolerance;
}zcl_temperatureAttr_t;

extern zcl_temperatureAttr_t g_zcl_temperatureAttrs;

extern uint8_t APP_CB_CLUSTER_NUM1;
//extern uint8_t APP_CB_CLUSTER_NUM2;
extern const zcl_specClusterInfo_t  g_appClusterList1[];
//extern const zcl_specClusterInfo_t  g_appClusterList2[];
extern const af_simple_descriptor_t app_ep1_simpleDesc;
//extern const af_simple_descriptor_t app_ep2_simpleDesc;

/* Attributes */
//extern zcl_basicAttr_t              g_zcl_basicAttrs;
extern zcl_identifyAttr_t           g_zcl_identifyAttrs;
extern zcl_groupAttr_t              g_zcl_groupAttrs;
extern zcl_sceneAttr_t              g_zcl_sceneAttrs;
extern zcl_onOffAttr_t              g_zcl_onOffAttrs;
extern zcl_msInputAttr_t            g_zcl_msInputAttrs;
extern zcl_seAttr_t                 g_zcl_seAttrs;
extern zcl_msAttr_t                 g_zcl_msAttrs;

#define zcl_onOffAttrsGet()         &g_zcl_onOffAttrs;
#define zcl_onOffCfgAttrsGet()      &g_zcl_onOffCfgAttrs;
#define zcl_msInputAttrsGet()       &g_zcl_msInputAttrs;
#define zcl_seAttrsGet()            &g_zcl_seAttrs;
#define zcl_msAttrsGet()            &g_zcl_msAttrs;

void populate_date_code(void);
nv_sts_t load_config_min_max(void);
nv_sts_t save_config_min_max(void);
nv_sts_t load_config_on_off(void);
nv_sts_t save_config_on_off(void);

#endif /* SRC_INCLUDE_APP_ENDPOINT_CFG_H_ */

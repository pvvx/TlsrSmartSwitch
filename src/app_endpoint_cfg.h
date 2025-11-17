#ifndef SRC_INCLUDE_APP_ENDPOINT_CFG_H_
#define SRC_INCLUDE_APP_ENDPOINT_CFG_H_

#define APP_ENDPOINT1 0x01
#define APP_ENDPOINT2 0x02
#define APP_ENDPOINT3 0x03

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


/* sys_mode:
0 - Cooling Only
1 - Cooling With Reheat
2 - Heating Only
3 - Heating With Reheat
4 - Cooling and Heating 4-pipes
5 - Cooling and Heating 4-pipes with Reheat */




typedef struct {
	int16_t temp_cooling;
	int16_t temp_heating;
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
	int16_t local_temp; // in 0.01 C
	int16_t min_temp; // in 0.01 C
	int16_t max_temp; // in 0.01 C
	uint8_t cool_on;
	uint8_t healt_on;
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
	// work
	uint32_t errors; // AC_ERROR_CODE
	int32_t summ_temp;
	uint8_t summ_cnt;
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
typedef struct {
    uint16_t onTime;
    uint16_t offWaitTime;
    uint8_t  startUpOnOff;
    bool     onOff;
    bool     globalSceneControl;
} zcl_onOffAttr_t;

typedef struct {
    uint8_t switchType;
    uint8_t switchActions;
    uint8_t custom_swtichType; // custom RW
    uint8_t custom_decoupled;
} zcl_onOffCfgAttr_t;

/**
 *  @brief Defined for multistate input clusters attributes
 */
typedef struct {
    uint8_t     out_of_service;
    uint16_t    value;
    uint16_t    num;
    uint8_t     status_flag;
} zcl_msInputAttr_t;

typedef struct {
    uint64_t cur_sum_delivered;
    uint32_t mutipler;
    uint32_t divisor;
    uint8_t  unit_of_measure;       // 0x00 - kWh
    uint8_t  status;
    uint8_t  summation_formatting;  // Bits 0 to 2: Number of Digits to the right of the Decimal Point
                                    // Bits 3 to 6: Number of Digits to the left of the Decimal Point
                                    // Bit  7:      If set, suppress leading zeros
    uint8_t  device_type;

} zcl_seAttr_t;

typedef struct {
    uint32_t type;
#if USE_BL0942
    uint16_t freq;
#endif
    uint16_t current;
    uint16_t voltage;
    uint16_t power;
    uint16_t mutipler;
    uint16_t divisor;
    uint16_t power_divisor;
    uint16_t current_divisor;
} zcl_msAttr_t;

typedef struct {
    int16_t max_voltage;
    int16_t min_voltage;
    int16_t max_current;
    uint16_t time_max_current;
    uint16_t time_reload;
    uint16_t time_start;
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
extern zcl_basicAttr_t              g_zcl_basicAttrs;
extern zcl_identifyAttr_t           g_zcl_identifyAttrs;
extern zcl_groupAttr_t              g_zcl_groupAttrs[];
extern zcl_sceneAttr_t              g_zcl_sceneAttrs[];
extern zcl_onOffAttr_t              g_zcl_onOffAttrs[];
extern zcl_onOffCfgAttr_t           g_zcl_onOffCfgAttrs[];
extern zcl_msInputAttr_t            g_zcl_msInputAttrs[];
extern zcl_seAttr_t                 g_zcl_seAttrs;
extern zcl_msAttr_t                 g_zcl_msAttrs;

#define zcl_groupAttrsGet()         g_zcl_groupAttrs
#define zcl_sceneAttrGet()          g_zcl_sceneAttrs
#define zcl_onOffAttrsGet()         g_zcl_onOffAttrs;
#define zcl_onOffCfgAttrsGet()      g_zcl_onOffCfgAttrs;
#define zcl_msInputAttrsGet()       g_zcl_msInputAttrs;
#define zcl_seAttrsGet()            &g_zcl_seAttrs;
#define zcl_msAttrsGet()            &g_zcl_msAttrs;

void populate_date_code(void);
nv_sts_t load_config_min_max(void);
nv_sts_t save_config_min_max(void);

#endif /* SRC_INCLUDE_APP_ENDPOINT_CFG_H_ */

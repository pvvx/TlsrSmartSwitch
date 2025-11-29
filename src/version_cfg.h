/************************************************************************************
 * @file    version_cfg.h
 *
 * @brief   This is the header file for version_cfg
 *
 * @author  Zigbee Group
 * @date    2021
 *
 * @par     Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *			All rights reserved.
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 ***********************************************************************************/
#ifndef _VERSION_CFG_H_
#define _VERSION_CFG_H_

#ifndef USE_BLE
#define USE_BLE			0
#endif

#define BOOT_LOADER_MODE					0

/* Boot loader address. */
#define BOOT_LOADER_IMAGE_ADDR				0x0

/* APP image address. */
#if (BOOT_LOADER_MODE)
	#define APP_IMAGE_ADDR					0x8000
#else
	#define APP_IMAGE_ADDR					0x0
#endif

/* Board ID */
#define BOARD_EM1SW1		0 // ZTU, BL0942
#define BOARD_EM1SW2		1 // ZTU, BL0942, ext.switch
#define BOARD_EM1SW1T		2 // ZTU, BL0942, MY18B20
#define BOARD_EM1SW2T		3 // ZTU, BL0942, ext.switch, MY18B20
#define BOARD_EM1SW1TS		6 // ZTU, BL0942, MY18B20, TERMOSTAT
#define BOARD_EM1SW2TS		7 // ZTU, BL0942, ext.switch, MY18B20, TERMOSTAT
                    
#define BOARD_EM8SW1		8 // ZTU, BL0942
#define BOARD_EM8SW2		9 // ZTU, BL0942, ext.switch
#define BOARD_EM8SW1T		10 // ZTU, BL0942, MY18B20
#define BOARD_EM8SW2T		11 // ZTU, BL0942, ext.switch, MY18B20
#define BOARD_EM8SW1TS		14 // ZTU, BL0942, MY18B20, TERMOSTAT
#define BOARD_EM8SW2TS		15 // ZTU, BL0942, ext.switch, MY18B20, TERMOSTAT

/* Board define */
#ifndef BOARD
#define BOARD				BOARD_EM8SW1TS
#endif

#if (BOARD & 8)
#define USE_BL0937			1
#define USE_BL0942			0
#else
#define USE_BL0937			0
#define USE_BL0942			1
#endif
#if (BOARD & 1)
#define USE_SWITCH			1
#else
#define USE_SWITCH			0
#endif
#if (BOARD & 2)
#define USE_SENSOR_MY18B20	1
#else
#define USE_SENSOR_MY18B20	0
#endif
#if (BOARD & 4)
#define USE_THERMOSTAT		1
#else
#define USE_THERMOSTAT		0
#endif

/* Chip IDs */
#define TLSR_8267				0x00
#define TLSR_8269				0x01
#define TLSR_8258_512K			0x02
#define TLSR_8258_1M			0x03
#define TLSR_8278				0x04
#define TLSR_B91				0x05

#if defined(MCU_CORE_826x)
	#if (CHIP_8269)
		#define CHIP_TYPE		TLSR_8269
	#else
		#define CHIP_TYPE		TLSR_8267
	#endif
#elif defined(MCU_CORE_8258)
		#define CHIP_TYPE		TLSR_8258_512K	//TLSR_8258_1M
#elif defined(MCU_CORE_8278)
		#define CHIP_TYPE		TLSR_8278
#elif defined(MCU_CORE_B91)
		#define CHIP_TYPE		TLSR_B91
#endif


#define APP_RELEASE				0x00	//0x12 -> BCD app release "1.2"
#define APP_BUILD				0x05	//0x34 -> BCD app build "3.4"

#define STACK_RELEASE			0x30	//BCD stack release 3.0
#define STACK_BUILD				0x01	//BCD stack build 01

/*********************************************************************************************
 * During OTA upgrade, the upgraded device will check the rules of the following three fields.
 * Refer to ZCL OTA specification for details.
 */
//#define MANUFACTURER_CODE_TELINK           	0x1141	// Telink ID
#define MANUFACTURER_CODE_TELINK           	0xEA60	// 60000 Telink-pvvx
#define	IMAGE_TYPE							((USE_BL0937 << 15) | (USE_BL0942 << 12) | (USE_THERMOSTAT << 8) | (USE_SENSOR_MY18B20 << 4)  | USE_SWITCH)
#define	FILE_VERSION					  	((APP_RELEASE << 24) | (APP_BUILD << 16) | (STACK_RELEASE << 8) | STACK_BUILD)

/* Pre-compiled link configuration. */
#define IS_BOOT_LOADER_IMAGE				0
#define RESV_FOR_APP_RAM_CODE_SIZE			0
#define IMAGE_OFFSET						APP_IMAGE_ADDR

#define	DEEPRETENTION_SECTION_USED          1  // (if BLE)

#endif // _VERSION_CFG_H_


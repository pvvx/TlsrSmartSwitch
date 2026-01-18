/**********************************************************************
 * @file    zcl_appCb.c
 *
 * @brief   This is the source file for zcl_appCb
 *
 *********************************************************************/

/**********************************************************************
 * INCLUDES
 */
#include "app_main.h"
#if USE_SENSOR_MY18B20
#include "my18b20.h"
#endif
/**********************************************************************
 * LOCAL CONSTANTS
 */



/**********************************************************************
 * TYPEDEFS
 */


/**********************************************************************
 * LOCAL FUNCTIONS
 */
#if 0 //def ZCL_READ
static void app_zclReadRspCmd(zclReadRspCmd_t *pReadRspCmd);
#endif
#ifdef ZCL_WRITE
static void app_zclWriteReqCmd(uint8_t epId, uint16_t clusterId, zclWriteCmd_t *pWriteReqCmd);
static void app_zclWriteRspCmd(zclWriteRspCmd_t *pWriteRspCmd);
#endif
#if 0 //def ZCL_REPORT
static void app_zclCfgReportCmd(uint8_t endPoint, uint16_t clusterId, zclCfgReportCmd_t *pCfgReportCmd);
static void app_zclCfgReportRspCmd(zclCfgReportRspCmd_t *pCfgReportRspCmd);
static void app_zclReportCmd(uint16_t clusterId, zclReportCmd_t *pReportCmd, aps_data_ind_t aps_data_ind);
#endif
static void app_zclDfltRspCmd(zclDefaultRspCmd_t *pDftRspCmd);


/**********************************************************************
 * GLOBAL VARIABLES
 */


/**********************************************************************
 * LOCAL VARIABLES
 */
#ifdef ZCL_IDENTIFY
static ev_timer_event_t *identifyTimerEvt = NULL;
#endif

uint8_t count_no_service = 0;


/**********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      app_zclProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message.
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  None
 */
void app_zclProcessIncomingMsg(zclIncoming_t *pInHdlrMsg)
{

//    uint16_t cluster = pInHdlrMsg->msg->indInfo.cluster_id;
    uint8_t endPoint = pInHdlrMsg->msg->indInfo.dst_ep;
//    aps_data_ind_t aps_data_ind = pInHdlrMsg->msg->indInfo;

    switch(pInHdlrMsg->hdr.cmd)
    {
#if 0 //def ZCL_READ
        case ZCL_CMD_READ_RSP:
            app_zclReadRspCmd(pInHdlrMsg->attrCmd);
            break;
#endif
#ifdef ZCL_WRITE
        case ZCL_CMD_WRITE:
            app_zclWriteReqCmd(endPoint, pInHdlrMsg->msg->indInfo.cluster_id, pInHdlrMsg->attrCmd);
            break;
        case ZCL_CMD_WRITE_RSP:
            app_zclWriteRspCmd(pInHdlrMsg->attrCmd);
            break;
#endif
#if 0 //def ZCL_REPORT
        case ZCL_CMD_CONFIG_REPORT:
            app_zclCfgReportCmd(endPoint, cluster, pInHdlrMsg->attrCmd);
            break;
        case ZCL_CMD_CONFIG_REPORT_RSP:
            app_zclCfgReportRspCmd(pInHdlrMsg->attrCmd);
            break;
        case ZCL_CMD_REPORT:
            app_zclReportCmd(cluster, pInHdlrMsg->attrCmd, aps_data_ind);
            break;
#endif
        case ZCL_CMD_DEFAULT_RSP:
            app_zclDfltRspCmd(pInHdlrMsg->attrCmd);
            break;
        default:
            break;
    }
}

#if 0 // def ZCL_READ
/*********************************************************************
 * @fn      app_zclReadRspCmd
 *
 * @brief   Handler for ZCL Read Response command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void app_zclReadRspCmd(zclReadRspCmd_t *pReadRspCmd) {

    uint8_t numAttr = pReadRspCmd->numAttr;
    zclReadRspStatus_t *attrList = pReadRspCmd->attrList;
    uint32_t time_local;
    bool time_sent = false;

    for (uint8_t i = 0; i < numAttr; i++) {
        if (attrList[i].attrID == ZCL_ATTRID_LOCAL_TIME && attrList[i].status == ZCL_STA_SUCCESS) {
            time_local = attrList[i].data[0] & 0xff;
            time_local |= (attrList[i].data[1] << 8)  & 0x0000ffff;
            time_local |= (attrList[i].data[2] << 16) & 0x00ffffff;
            time_local |= (attrList[i].data[3] << 24) & 0xffffffff;
            zcl_setAttrVal(APP_ENDPOINT1, ZCL_CLUSTER_GEN_TIME, ZCL_ATTRID_LOCAL_TIME, (uint8_t*)&time_local);
            time_sent = true;
#if UART_PRINTF_MODE && DEBUG_TIME
            printf("Sync Local Time: %d\r\n", time_local+UNIX_TIME_CONST);
#endif
        }
    }

    if (time_sent) {
//        set_time_sent();
    }
}
#endif

#ifdef ZCL_WRITE

enum {
	NBIT_ON_OFF_CONFIG = 1,
	NBIT_MIN_MAX_CONFIG,
	NBIT_SENSOR_CONFIG,
	NBIT_SENSOR_CALIBRATE,
	NBIT_MY18B20_CONFIG,
	NBIT_THERM_CONFIG,
	NBIT_GPIO_CONFIG
};
/*********************************************************************
 * @fn      app_zclWriteReqCmd
 *
 * @brief   Handler for ZCL Write Request command.
 *
 * @param
 *
 * @return  None
 */
static void app_zclWriteReqCmd(uint8_t epId, uint16_t clusterId, zclWriteCmd_t *pWriteReqCmd)
{
    zclWriteRec_t *attr = pWriteReqCmd->attrList;
    uint32_t save = 0;
    uint16_t attrID;
    uint8_t data;
    uint8_t numAttr = pWriteReqCmd->numAttr;
    for (uint32_t i = 0; i < numAttr; i++) {
    	data = attr[i].attrData[0];
    	attrID = attr[i].attrID;
		if (clusterId == ZCL_CLUSTER_GEN_ON_OFF) {
#if USE_CFG_GPIO
			if(attrID >= ZCL_ATTRID_GPIO_RELAY) {
				save |= BIT(NBIT_GPIO_CONFIG);
			} else
#endif
			if (attrID >= ZCL_ATTRID_START_UP_ONOFF) {
				save |= BIT(NBIT_ON_OFF_CONFIG);
			}
#if USE_SWITCH
		} else if (clusterId == ZCL_CLUSTER_GEN_ON_OFF_SWITCH_CONFIG) {
				save |= BIT(NBIT_ON_OFF_CONFIG);
#endif // USE_SWITCH
#if USE_SENSOR_MY18B20
 #ifdef ZCL_THERMOSTAT
        } else if (clusterId == ZCL_CLUSTER_HAVC_THERMOSTAT) {
#ifndef ZCL_TEMPERATURE_MEASUREMENT
        	if(attrID == ZCL_ATTRID_HVAC_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT
        		|| attrID == ZCL_ATTRID_HVAC_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT
				attrID >= ZCL_TEMPERATURE_SENSOR_MULTIPLER) {
        		save |= BIT(NBIT_MY18B20_CONFIG);
        	} else
#endif
        	{
       			save |= BIT(NBIT_THERM_CONFIG);
        	}
 #endif // ZCL_THERMOSTAT
 #ifdef ZCL_TEMPERATURE_MEASUREMENT
        } else if (clusterId == ZCL_CLUSTER_MS_TEMPERATURE_MEASUREMENT) {
       		save |= BIT(NBIT_MY18B20_CONFIG);
 #endif // ZCL_TEMPERATURE_MEASUREMENT
#endif // USE_SENSOR_MY18B20
#ifdef ZCL_ELECTRICAL_MEASUREMENT
		} else if (clusterId == ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT) {
			if (attrID >= ZCL_ATTRID_CURRENT_COEF
				&& attrID <= ZCL_ATTRID_FGREQ_COEF) {
				save |= BIT(NBIT_SENSOR_CONFIG);
#if USE_CALIBRATE_CVP
			} else if (attrID >= ZCL_ATTRID_CURRENT_CAL
				&& attrID <= ZCL_ATTRID_POWER_CAL) {
				save |= BIT(NBIT_SENSOR_CALIBRATE);
#endif
			} else {
				save |= BIT(NBIT_MIN_MAX_CONFIG);
			}
#endif // ZCL_ELECTRICAL_MEASUREMENT
		}
	}
#if USE_CALIBRATE_CVP
    if (save & BIT(NBIT_SENSOR_CALIBRATE)) {
    	check_start_calibrate();
    }
#endif
#if USE_METERING
    if (save & BIT(NBIT_SENSOR_CONFIG)) {
    	save_config_sensor();
    }
    if (save & BIT(NBIT_MIN_MAX_CONFIG)) {
    	save_config_min_max();
    }
#endif
#if USE_SENSOR_MY18B20
 #ifdef ZCL_THERMOSTAT
    if (save &  BIT(NBIT_THERM_CONFIG)) {
    	save_config_termostat();
    	// restore relay
#if USE_SENSOR_MY18B20
    	set_therm_relay_status(cfg_on_off.onOff);
#else
		set_relay_status(cfg_on_off.onOff);
#endif
    }
 #endif
    if (save &  BIT(NBIT_MY18B20_CONFIG)) {
    	save_config_my18b20();
    }
#endif // USE_SENSOR_MY18B20

    if (save & BIT(NBIT_ON_OFF_CONFIG)) {
    	save_config_on_off();
//    	led_set_control();	// restore led?
    	// restore relay
#if USE_THERMOSTAT // USE_SENSOR_MY18B20
    	set_therm_relay_status(cfg_on_off.onOff);
#else
		set_relay_status(cfg_on_off.onOff);
#endif
    }
#if USE_CFG_GPIO
    if (save & BIT(NBIT_GPIO_CONFIG)) {
    	save_config_gpio();
    }
#endif
#ifdef ZCL_POLL_CTRL
    if(clusterId == ZCL_CLUSTER_GEN_POLL_CONTROL){
        for(int32_t i = 0; i < numAttr; i++){
            if(attr[i].attrID == ZCL_ATTRID_CHK_IN_INTERVAL){
                app_zclCheckInStart();
                return;
            }
        }
    }
#endif
}

/*********************************************************************
 * @fn      app_zclWriteRspCmd
 *
 * @brief   Handler for ZCL Write Response command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void app_zclWriteRspCmd(zclWriteRspCmd_t *pWriteRspCmd) {}
#endif


/*********************************************************************
 * @fn      app_zclDfltRspCmd
 *
 * @brief   Handler for ZCL Default Response command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void app_zclDfltRspCmd(zclDefaultRspCmd_t *pDftRspCmd)
{
//  printf("app_zclDfltRspCmd\n");
#ifdef ZCL_OTA
    if( (pDftRspCmd->commandID == ZCL_CMD_OTA_UPGRADE_END_REQ) &&
        (pDftRspCmd->statusCode == ZCL_STA_ABORT) ){
        if(zcl_attr_imageUpgradeStatus == IMAGE_UPGRADE_STATUS_DOWNLOAD_COMPLETE){
            ota_upgradeAbort();
        }
    }
#endif
}

#if 0 //def ZCL_REPORT
/*********************************************************************
 * @fn      app_zclCfgReportCmd
 *
 * @brief   Handler for ZCL Configure Report command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void app_zclCfgReportCmd(uint8_t endPoint, uint16_t clusterId, zclCfgReportCmd_t *pCfgReportCmd)
{
    //printf("app_zclCfgReportCmd\r\n");
    reportAttrTimerStop();
}

/*********************************************************************
 * @fn      app_zclCfgReportRspCmd
 *
 * @brief   Handler for ZCL Configure Report Response command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void app_zclCfgReportRspCmd(zclCfgReportRspCmd_t *pCfgReportRspCmd)
{
//    printf("app_zclCfgReportRspCmd\n");

}

/*********************************************************************
 * @fn      app_zclReportCmd
 *
 * @brief   Handler for ZCL Report command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void app_zclReportCmd(uint16_t clusterId, zclReportCmd_t *pReportCmd, aps_data_ind_t aps_data_ind) {
//    printf("app_zclReportCmd\r\n");

//    uint8_t numAttr = pReportCmd->numAttr;
//    zclReport_t *attrList = pReportCmd->attrList;
//
//    uint8_t ret;
//    uint16_t addr = aps_data_ind.src_short_addr;
//
//    for (uint8_t i = 0; i < numAttr; i++) {
//        if (clusterId == ZCL_CLUSTER_MS_TEMPERATURE_MEASUREMENT &&
//                attrList[i].dataType == ZCL_DATA_TYPE_INT16 &&
//                attrList[i].attrID == ZCL_TEMPERATURE_MEASUREMENT_ATTRID_MEASUREDVALUE) {
//            int16_t temp;
//
//            temp = attrList[i].attrData[0] & 0xFF;
//            temp |= (attrList[i].attrData[1] << 8) & 0xFFFF;
//
////            printf("temp: 0x%04x\r\n", (uint16_t)temp);
//
//            ret = bind_outsise_proc(addr, clusterId);
//
//            if (ret != OUTSIDE_SRC_CLUSTER_OK) {
//                continue;
//            }
//
//            app_set_remote_temperature(temp);
//            bind_remote_update_timer();
//        }
//    }

}
#endif

#ifdef ZCL_BASIC
/*********************************************************************
 * @fn      app_basicCb
 *
 * @brief   Handler for ZCL Basic Reset command.
 *
 * @param   pAddrInfo
 * @param   cmdId
 * @param   cmdPayload
 *
 * @return  status_t
 */
status_t app_basicCb(zclIncomingAddrInfo_t *pAddrInfo, uint8_t cmdId, void *cmdPayload)
{
    if(cmdId == ZCL_CMD_BASIC_RESET_FAC_DEFAULT){
        //Reset all the attributes of all its clusters to factory defaults
        //zcl_nv_attr_reset();
    }

    return ZCL_STA_SUCCESS;
}
#endif

#ifdef ZCL_IDENTIFY
int32_t app_zclIdentifyTimerCb(void *arg)
{
    if(g_zcl_identifyAttrs.identifyTime <= 0){
        light_blink_stop();

        identifyTimerEvt = NULL;
        return -1;
    }
    g_zcl_identifyAttrs.identifyTime--;
    return 0;
}

void app_zclIdentifyTimerStop(void)
{
    if(identifyTimerEvt){
        TL_ZB_TIMER_CANCEL(&identifyTimerEvt);
    }
}

/*********************************************************************
 * @fn      app_zclIdentifyCmdHandler
 *
 * @brief   Handler for ZCL Identify command. This function will set blink LED.
 *
 * @param   endpoint
 * @param   srcAddr
 * @param   identifyTime - identify time
 *
 * @return  None
 */
void app_zclIdentifyCmdHandler(uint8_t endpoint, uint16_t srcAddr, uint16_t identifyTime)
{
    g_zcl_identifyAttrs.identifyTime = identifyTime;

    if(identifyTime == 0){
        app_zclIdentifyTimerStop();
        light_blink_stop();
    }else{
        if(!identifyTimerEvt){
            light_blink_start(identifyTime, 500, 500);
            identifyTimerEvt = TL_ZB_TIMER_SCHEDULE(app_zclIdentifyTimerCb, NULL, 1000);
        }
    }
}

/*********************************************************************
 * @fn      app_zcltriggerCmdHandler
 *
 * @brief   Handler for ZCL trigger command.
 *
 * @param   pTriggerEffect
 *
 * @return  None
 */
static void app_zcltriggerCmdHandler(zcl_triggerEffect_t *pTriggerEffect)
{
    uint8_t effectId = pTriggerEffect->effectId;
//  uint8_t effectVariant = pTriggerEffect->effectVariant;


    switch (effectId) {
        case IDENTIFY_EFFECT_BLINK:
            light_blink_start(1, 500, 500);
            break;
        case IDENTIFY_EFFECT_BREATHE:
            light_blink_start(15, 300, 700);
            break;
        case IDENTIFY_EFFECT_OKAY:
            light_blink_start(2, 250, 250);
            break;
        case IDENTIFY_EFFECT_CHANNEL_CHANGE:
            light_blink_start(1, 500, 7500);
            break;
        case IDENTIFY_EFFECT_FINISH_EFFECT:
            light_blink_start(1, 300, 700);
            break;
        case IDENTIFY_EFFECT_STOP_EFFECT:
            light_blink_stop();
            break;
        default:
            break;
    }
}

/*********************************************************************
 * @fn      app_identifyCb
 *
 * @brief   Handler for ZCL Identify command.
 *
 * @param   pAddrInfo
 * @param   cmdId
 * @param   cmdPayload
 *
 * @return  status_t
 */
status_t app_identifyCb(zclIncomingAddrInfo_t *pAddrInfo, uint8_t cmdId, void *cmdPayload)
{
    if(pAddrInfo->dstEp == APP_ENDPOINT1){
        if(pAddrInfo->dirCluster == ZCL_FRAME_CLIENT_SERVER_DIR){
            switch(cmdId){
                case ZCL_CMD_IDENTIFY:
                    app_zclIdentifyCmdHandler(pAddrInfo->dstEp, pAddrInfo->srcAddr, ((zcl_identifyCmd_t *)cmdPayload)->identifyTime);
                    break;
                case ZCL_CMD_TRIGGER_EFFECT:
                    app_zcltriggerCmdHandler((zcl_triggerEffect_t *)cmdPayload);
                    break;
                default:
                    break;
            }
        }
    }

    return ZCL_STA_SUCCESS;
}
#endif

#ifdef ZCL_GROUP
/*********************************************************************
 * @fn      app_zclAddGroupRspCmdHandler
 *
 * @brief   Handler for ZCL add group response command.
 *
 * @param   pAddGroupRsp
 *
 * @return  None
 */
static void app_zclAddGroupRspCmdHandler(uint8_t ep, zcl_addGroupRsp_t *pAddGroupRsp) {

//    printf("app_zclAddGroupRspCmdHandler. ep: %d, status: %d, gid: %d\r\n", ep, pAddGroupRsp->status, pAddGroupRsp->groupId);
}

/*********************************************************************
 * @fn      app_zclViewGroupRspCmdHandler
 *
 * @brief   Handler for ZCL view group response command.
 *
 * @param   pViewGroupRsp
 *
 * @return  None
 */
static void app_zclViewGroupRspCmdHandler(zcl_viewGroupRsp_t *pViewGroupRsp)
{

}

/*********************************************************************
 * @fn      app_zclRemoveGroupRspCmdHandler
 *
 * @brief   Handler for ZCL remove group response command.
 *
 * @param   pRemoveGroupRsp
 *
 * @return  None
 */
static void app_zclRemoveGroupRspCmdHandler(zcl_removeGroupRsp_t *pRemoveGroupRsp)
{

}

/*********************************************************************
 * @fn      app_zclGetGroupMembershipRspCmdHandler
 *
 * @brief   Handler for ZCL get group membership response command.
 *
 * @param   pGetGroupMembershipRsp
 *
 * @return  None
 */
static void app_zclGetGroupMembershipRspCmdHandler(zcl_getGroupMembershipRsp_t *pGetGroupMembershipRsp)
{

}

/*********************************************************************
 * @fn      app_groupCb
 *
 * @brief   Handler for ZCL Group command.
 *
 * @param   pAddrInfo
 * @param   cmdId
 * @param   cmdPayload
 *
 * @return  status_t
 */
status_t app_groupCb(zclIncomingAddrInfo_t *pAddrInfo, uint8_t cmdId, void *cmdPayload) {

//    printf("app_groupCb. ep: %d\r\n", pAddrInfo->dstEp);

	if(pAddrInfo->dstEp == APP_ENDPOINT1) {
		if(pAddrInfo->dirCluster == ZCL_FRAME_SERVER_CLIENT_DIR){
			switch(cmdId){
				case ZCL_CMD_GROUP_ADD_GROUP_RSP:
					app_zclAddGroupRspCmdHandler(pAddrInfo->dstEp, (zcl_addGroupRsp_t *)cmdPayload);
					break;
				case ZCL_CMD_GROUP_VIEW_GROUP_RSP:
					app_zclViewGroupRspCmdHandler((zcl_viewGroupRsp_t *)cmdPayload);
					break;
				case ZCL_CMD_GROUP_REMOVE_GROUP_RSP:
					app_zclRemoveGroupRspCmdHandler((zcl_removeGroupRsp_t *)cmdPayload);
					break;
				case ZCL_CMD_GROUP_GET_MEMBERSHIP_RSP:
					app_zclGetGroupMembershipRspCmdHandler((zcl_getGroupMembershipRsp_t *)cmdPayload);
					break;
				default:
					break;
			}
		}
	}

	return ZCL_STA_SUCCESS;
}
#endif	/* ZCL_GROUP */

#ifdef ZCL_SCENE
/*********************************************************************
 * @fn      app_zclAddSceneRspCmdHandler
 *
 * @brief   Handler for ZCL add scene response command.
 *
 * @param   cmdId
 * @param   pAddSceneRsp
 *
 * @return  None
 */
static void app_zclAddSceneRspCmdHandler(uint8_t cmdId, addSceneRsp_t *pAddSceneRsp)
{

}

/*********************************************************************
 * @fn      app_zclViewSceneRspCmdHandler
 *
 * @brief   Handler for ZCL view scene response command.
 *
 * @param   cmdId
 * @param   pViewSceneRsp
 *
 * @return  None
 */
static void app_zclViewSceneRspCmdHandler(uint8_t cmdId, viewSceneRsp_t *pViewSceneRsp)
{

}

/*********************************************************************
 * @fn      app_zclRemoveSceneRspCmdHandler
 *
 * @brief   Handler for ZCL remove scene response command.
 *
 * @param   pRemoveSceneRsp
 *
 * @return  None
 */
static void app_zclRemoveSceneRspCmdHandler(removeSceneRsp_t *pRemoveSceneRsp)
{

}

/*********************************************************************
 * @fn      app_zclRemoveAllSceneRspCmdHandler
 *
 * @brief   Handler for ZCL remove all scene response command.
 *
 * @param   pRemoveAllSceneRsp
 *
 * @return  None
 */
static void app_zclRemoveAllSceneRspCmdHandler(removeAllSceneRsp_t *pRemoveAllSceneRsp)
{

}

/*********************************************************************
 * @fn      app_zclStoreSceneRspCmdHandler
 *
 * @brief   Handler for ZCL store scene response command.
 *
 * @param   pStoreSceneRsp
 *
 * @return  None
 */
static void app_zclStoreSceneRspCmdHandler(storeSceneRsp_t *pStoreSceneRsp)
{

}

/*********************************************************************
 * @fn      app_zclGetSceneMembershipRspCmdHandler
 *
 * @brief   Handler for ZCL get scene membership response command.
 *
 * @param   pGetSceneMembershipRsp
 *
 * @return  None
 */
static void app_zclGetSceneMembershipRspCmdHandler(getSceneMemRsp_t *pGetSceneMembershipRsp)
{

}

/*********************************************************************
 * @fn      app_sceneCb
 *
 * @brief   Handler for ZCL Scene command.
 *
 * @param   pAddrInfo
 * @param   cmdId
 * @param   cmdPayload
 *
 * @return  status_t
 */
status_t app_sceneCb(zclIncomingAddrInfo_t *pAddrInfo, uint8_t cmdId, void *cmdPayload)
{
	if(pAddrInfo->dstEp == APP_ENDPOINT1){
		if(pAddrInfo->dirCluster == ZCL_FRAME_SERVER_CLIENT_DIR){
			switch(cmdId){
				case ZCL_CMD_SCENE_ADD_SCENE_RSP:
				case ZCL_CMD_SCENE_ENHANCED_ADD_SCENE_RSP:
					app_zclAddSceneRspCmdHandler(cmdId, (addSceneRsp_t *)cmdPayload);
					break;
				case ZCL_CMD_SCENE_VIEW_SCENE_RSP:
				case ZCL_CMD_SCENE_ENHANCED_VIEW_SCENE_RSP:
					app_zclViewSceneRspCmdHandler(cmdId, (viewSceneRsp_t *)cmdPayload);
					break;
				case ZCL_CMD_SCENE_REMOVE_SCENE_RSP:
					app_zclRemoveSceneRspCmdHandler((removeSceneRsp_t *)cmdPayload);
					break;
				case ZCL_CMD_SCENE_REMOVE_ALL_SCENE_RSP:
					app_zclRemoveAllSceneRspCmdHandler((removeAllSceneRsp_t *)cmdPayload);
					break;
				case ZCL_CMD_SCENE_STORE_SCENE_RSP:
					app_zclStoreSceneRspCmdHandler((storeSceneRsp_t *)cmdPayload);
					break;
				case ZCL_CMD_SCENE_GET_SCENE_MEMSHIP_RSP:
					app_zclGetSceneMembershipRspCmdHandler((getSceneMemRsp_t *)cmdPayload);
					break;
				default:
					break;
			}
		}
	}

	return ZCL_STA_SUCCESS;
}
#endif	/* ZCL_SCENE */



status_t app_onOffCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload) {

//    printf("app_onOffCb, dstEp: %d\r\n", pAddrInfo->dstEp);

    zcl_onOffAttr_t *pOnOff = zcl_onOffAttrsGet();
//    pOnOff += pAddrInfo->dstEp - 1;

    if(pAddrInfo->dstEp == APP_ENDPOINT1 || pAddrInfo->dstEp == APP_ENDPOINT2) {
    	switch(cmdId){
    		case ZCL_CMD_ONOFF_ON:
    			cmdOnOff_on();
                break;
    		case ZCL_CMD_ONOFF_OFF:
    			cmdOnOff_off();
    			break;
    		case ZCL_CMD_ONOFF_TOGGLE:
    			cmdOnOff_toggle();
    			break;
    		case ZCL_CMD_ON_WITH_RECALL_GLOBAL_SCENE:
//    			if(pOnOff->globalSceneControl == FALSE){
                pOnOff->globalSceneControl = TRUE;
//    			}
    			break;
            default:
            	break;
            }
#if 0
        } else {
            cmdOnOff_off();
        }
#endif
    }

    return ZCL_STA_SUCCESS;
}


status_t app_msInputCb(zclIncomingAddrInfo_t *pAddrInfo, uint8_t cmdId, void *cmdPayload) {

//    printf("app_aInputCb(). pAddrInfo->dirCluster: %0x%x, cmdId: 0x%x\r\n", pAddrInfo->dirCluster, cmdId);

    status_t status = ZCL_STA_SUCCESS;

    return status;
}

#ifdef ZCL_METERING
/*********************************************************************
 * @fn      app_meteringCb
 *
 * @brief   Handler for ZCL Identify command.
 *
 * @param   pAddrInfo
 * @param   cmdId
 * @param   cmdPayload
 *
 * @return  status_t
 */
status_t app_meteringCb(zclIncomingAddrInfo_t *pAddrInfo, uint8_t cmdId, void *cmdPayload)
{

//    printf("app_meteringCb\r\n");
    return ZCL_STA_SUCCESS;
}
#endif


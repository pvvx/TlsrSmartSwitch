/************************************************************************************
 * @file    zb_appCb.c
 *
 * @brief   This is the source file for zb_appCb
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
/**********************************************************************
 * INCLUDES
 */
#include "app_main.h"
#include "energy_save.h"

/**********************************************************************
 * LOCAL CONSTANTS
 */

/**********************************************************************
 * TYPEDEFS
 */


/**********************************************************************
 * LOCAL FUNCTIONS
 */
void zb_bdbInitCb(uint8_t status, uint8_t joinedNetwork);
void zb_bdbCommissioningCb(uint8_t status, void *arg);
void zb_bdbIdentifyCb(uint8_t endpoint, uint16_t srcAddr, uint16_t identifyTime);
void zb_bdbFindBindSuccessCb(findBindDst_t *pDstInfo);

/**********************************************************************
 * GLOBAL VARIABLES
 */
bdb_appCb_t g_zbBdbCb = {
    zb_bdbInitCb,
    zb_bdbCommissioningCb,
    zb_bdbIdentifyCb,
	zb_bdbFindBindSuccessCb
};

#ifdef ZCL_OTA
ota_callBack_t app_otaCb = {
    app_otaProcessMsgHandler,
};
#endif

/**********************************************************************
 * LOCAL VARIABLES
 */
static ev_timer_event_t *rejoinBackoffTimerEvt = NULL;
static ev_timer_event_t *steerTimerEvt = NULL;

/**********************************************************************
 * FUNCTIONS
 */
static s32 app_bdbNetworkSteerStart(void *arg)
{
    bdb_networkSteerStart();

    steerTimerEvt = NULL;
    return -1;
}

#if FIND_AND_BIND_SUPPORT
static s32 app_bdbFindAndBindStart(void *arg)
{
    bdb_findAndBindStart(BDB_COMMISSIONING_ROLE_TARGET);

    return -1;
}
#endif

static s32 app_rejoinBackoff(void *arg)
{
    static bool rejoinMode = REJOIN_SECURITY;

    if (zb_isDeviceFactoryNew()) {
        rejoinBackoffTimerEvt = NULL;
        return -1;
    }

    //printf("rejoin mode = %d\n", rejoinMode);

    zb_rejoinSecModeSet(rejoinMode);
    zb_rejoinReq(zb_apsChannelMaskGet(), g_bdbAttrs.scanDuration);

    rejoinMode = !rejoinMode;

    return 0;
}

/*********************************************************************
 * @fn      zb_bdbInitCb
 *
 * @brief   application callback for bdb initiation
 *
 * @param   status - the status of bdb init BDB_INIT_STATUS_SUCCESS or BDB_INIT_STATUS_FAILURE
 *
 * @param   joinedNetwork  - 1: node is on a network, 0: node isn't on a network
 *
 * @return  None
 */
void zb_bdbInitCb(u8 status, u8 joinedNetwork)
{
    //printf("bdbInitCb: sta = %x, joined = %x\n", status, joinedNetwork);

    if (status == BDB_INIT_STATUS_SUCCESS) {
        /*
         * start bdb commissioning
         * */
        if (joinedNetwork) {

#ifdef ZCL_OTA
            ota_queryStart(OTA_PERIODIC_QUERY_INTERVAL);
#endif
        } else {

#if (!ZBHCI_EN)
            u16 jitter = 0;
            do {
                jitter = zb_random() % 0x0fff;
            } while (jitter == 0);

            if (steerTimerEvt) {
                TL_ZB_TIMER_CANCEL(&steerTimerEvt);
            }
            steerTimerEvt = TL_ZB_TIMER_SCHEDULE(app_bdbNetworkSteerStart, NULL, jitter);
#endif
        }
    } else {

    }
}
/*********************************************************************
 * @fn      zb_bdbCommissioningCb
 *
 * @brief   application callback for bdb commissioning
 *
 * @param   status - the status of bdb commissioning
 *
 * @param   arg
 *
 * @return  None
 */
void zb_bdbCommissioningCb(uint8_t status, void *arg)
{
    switch (status) {
    case BDB_COMMISSION_STA_SUCCESS:

        if (steerTimerEvt) {
            TL_ZB_TIMER_CANCEL(&steerTimerEvt);
        }

        if (rejoinBackoffTimerEvt) {
            TL_ZB_TIMER_CANCEL(&rejoinBackoffTimerEvt);
        }


#ifdef ZCL_OTA
        ota_queryStart(OTA_PERIODIC_QUERY_INTERVAL);
#endif

#if FIND_AND_BIND_SUPPORT
        if (!gLightCtx.bdbFindBindFlg) {
            gLightCtx.bdbFindBindFlg = TRUE;
            TL_ZB_TIMER_SCHEDULE(app_bdbFindAndBindStart, NULL, 1000);
        }
#endif
        sws_printf("bdb:bind\n");
        light_blink_start(7, 200, 200);
        break;
    case BDB_COMMISSION_STA_IN_PROGRESS:
        break;
    case BDB_COMMISSION_STA_NOT_AA_CAPABLE:
        break;
    case BDB_COMMISSION_STA_NO_NETWORK:
    case BDB_COMMISSION_STA_TCLK_EX_FAILURE:
    case BDB_COMMISSION_STA_TARGET_FAILURE:
        {
            sws_printf("bdb:fault\n");
            uint16_t jitter = 0;
            do {
                jitter = zb_random() % 0x2710;
            } while (jitter < 5000);

            if (steerTimerEvt) {
                TL_ZB_TIMER_CANCEL(&steerTimerEvt);
            }
            steerTimerEvt = TL_ZB_TIMER_SCHEDULE(app_bdbNetworkSteerStart, NULL, jitter);
            light_blink_start(5, 500, 500);
        }
        break;
    case BDB_COMMISSION_STA_FORMATION_FAILURE:
        break;
    case BDB_COMMISSION_STA_NO_IDENTIFY_QUERY_RESPONSE:
        break;
    case BDB_COMMISSION_STA_BINDING_TABLE_FULL:
        break;
    case BDB_COMMISSION_STA_NO_SCAN_RESPONSE:
        break;
    case BDB_COMMISSION_STA_NOT_PERMITTED:
        break;
    case BDB_COMMISSION_STA_REJOIN_FAILURE:
        sws_printf("bdb:rejion1\n");
        if (!rejoinBackoffTimerEvt) {
            rejoinBackoffTimerEvt = TL_ZB_TIMER_SCHEDULE(app_rejoinBackoff, NULL, 60 * 1000);
        }
        light_blink_start(5, 500, 500);
        break;
    case BDB_COMMISSION_STA_FORMATION_DONE:
#ifndef ZBHCI_EN
        tl_zbMacChannelSet(DEFAULT_CHANNEL);  //set default channel
#endif
        break;
    default:
        break;
    }
}
void zb_bdbIdentifyCb(uint8_t endpoint, uint16_t srcAddr, uint16_t identifyTime)
{
#if FIND_AND_BIND_SUPPORT
    extern void app_zclIdentifyCmdHandler(uint8_t endpoint, uint16_t srcAddr, uint16_t identifyTime);
    app_zclIdentifyCmdHandler(endpoint, srcAddr, identifyTime);
#endif
}

/*********************************************************************
 * @fn      zb_bdbFindBindSuccessCb
 *
 * @brief   application callback for finding & binding
 *
 * @param   pDstInfo
 *
 * @return  None
 */
void zb_bdbFindBindSuccessCb(findBindDst_t *pDstInfo)
{
#if FIND_AND_BIND_SUPPORT
    epInfo_t dstEpInfo;
    TL_SETSTRUCTCONTENT(dstEpInfo, 0);

    dstEpInfo.dstAddrMode = APS_SHORT_DSTADDR_WITHEP;
    dstEpInfo.dstAddr.shortAddr = pDstInfo->addr;
    dstEpInfo.dstEp = pDstInfo->endpoint;
    dstEpInfo.profileId = HA_PROFILE_ID;

    zcl_identify_identifyCmd(APP_ENDPOINT, &dstEpInfo, FALSE, 0, 0);
#endif
}

#ifdef ZCL_OTA
void app_otaProcessMsgHandler(uint8_t evt, uint8_t status)
{
    if (evt == OTA_EVT_START) {
        if (status == ZCL_STA_SUCCESS) {

        } else {

        }
    } else if (evt == OTA_EVT_COMPLETE) {
        if (status == ZCL_STA_SUCCESS) {
            ota_mcuReboot();
        } else {
            ota_queryStart(OTA_PERIODIC_QUERY_INTERVAL);
        }
    }
}
#endif

s32 app_softReset(void *arg)
{
    SYSTEM_RESET();

    return -1;
}

/*********************************************************************
 * @fn      app_leaveCnfHandler
 *
 * @brief   Handler for ZDO Leave Confirm message.
 *
 * @param   pRsp - parameter of leave confirm
 *
 * @return  None
 */
void app_leaveCnfHandler(nlme_leave_cnf_t *pLeaveCnf)
{
    if(pLeaveCnf->status == SUCCESS){
    	light_blink_start(3, 200, 200);

    	//waiting blink over
    	TL_ZB_TIMER_SCHEDULE(app_softReset, NULL, 2 * 1000);
    }
}

/*********************************************************************
 * @fn      app_leaveIndHandler
 *
 * @brief   Handler for ZDO leave indication message.
 *
 * @param   pInd - parameter of leave indication
 *
 * @return  None
 */
void app_leaveIndHandler(nlme_leave_ind_t *pLeaveInd)
{

}

/*********************************************************************
 *
 * @brief   Receive notification of PAN ID conflict.
 *
 * @param   pNwkUpdateCmd - Conflicting PAN ID information
 *
 * @return  TRUE  - Allow PAN ID conflict handling
 *          FALSE - Truncate the execution of PAN ID conflict handling
 */
bool app_nwkUpdateIndicateHandler(nwkCmd_nwkUpdate_t *pNwkUpdate)
{
    return FALSE;
}

/*********************************************************************
 * @fn      app_nwkStatusIndHandler
 *
 * @brief   Handler for NWK status indication message.
 *
 * @param   pInd - parameter of NWK status indication
 *
 * @return  None
 */
void app_nwkStatusIndHandler(zdo_nwk_status_ind_t *pNwkStatusInd)
{
    //printf("nwkStatusIndHandler: addr = %x, status = %x\n", pNwkStatusInd->shortAddr, pNwkStatusInd->status);

    if (pNwkStatusInd->status == NWK_COMMAND_STATUS_BAD_FRAME_COUNTER) {
        tl_zb_normal_neighbor_entry_t *nbe = nwk_neTblGetByShortAddr(pNwkStatusInd->shortAddr);
        if (nbe) {
            //printf("curFC = %d, rcvFC = %d, failCnt = %d\n", nbe->incomingFrameCnt, nbe->receivedFrameCnt, nbe->frameCounterFailCnt);

            /*
             * When the network does not support the network key update feature,
             * this is a barbaric method to solve the decryption failure problem
             * caused by Frame Counter overflow, but it may also brings
             * the hidden danger of being attacked.
             */
#if 1
            if ((nbe->frameCounterFailCnt >= 10) && (nbe->receivedFrameCnt < nbe->incomingFrameCnt)) {
                nbe->incomingFrameCnt = 0;
            }
#endif
        }
    } else if (pNwkStatusInd->status == NWK_COMMAND_STATUS_BAD_KEY_SEQUENCE_NUMBER) {
        zb_rejoinSecModeSet(REJOIN_INSECURITY);
        zb_rejoinReq(zb_apsChannelMaskGet(), g_bdbAttrs.scanDuration);
    }
}


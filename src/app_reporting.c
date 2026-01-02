#include "app_main.h"

#ifdef ZCL_MULTISTATE_INPUT
uint32_t last_timeReportMsi;     // time of the last attribute report ZCL_MULTISTATE_INPUT_ATTRID_PRESENT_VALUE
uint8_t  last_seqNum;

static int32_t resetMsiTimerCb(void *args) {

    zcl_msInputAttr_t *msInputAttr = zcl_msInputAttrsGet();
    msInputAttr->value = ACTION_EMPTY;
    return -1;
}
#endif

void app_forcedReport(uint8_t endpoint, uint16_t claster_id, uint16_t attr_id) {

    if (zb_isDeviceJoinedNwk()) {

        epInfo_t dstEpInfo;
        TL_SETSTRUCTCONTENT(dstEpInfo, 0);

        status_t ret = 0;
        dstEpInfo.profileId = HA_PROFILE_ID;
        dstEpInfo.dstAddrMode = APS_DSTADDR_EP_NOTPRESETNT;

        zclAttrInfo_t *pAttrEntry = zcl_findAttribute(endpoint, claster_id, attr_id);

        if (!pAttrEntry) {
            //should not happen.
            ZB_EXCEPTION_POST(SYS_EXCEPTTION_ZB_ZCL_ENTRY);
            return;
        }
#ifdef ZCL_MULTISTATE_INPUT
        if (attr_id == ZCL_MULTISTATE_INPUT_ATTRID_PRESENT_VALUE) {
            last_timeReportMsi = clock_time();
            last_seqNum = ZCL_SEQ_NUM;
            ret = zcl_report(endpoint, &dstEpInfo, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR, last_seqNum,
                    MANUFACTURER_CODE_NONE, claster_id, pAttrEntry->id, pAttrEntry->type, pAttrEntry->data);
            TL_ZB_TIMER_SCHEDULE(resetMsiTimerCb, (void*)((uint32_t)endpoint), TIMEOUT_750MS);
        } else
#endif
        {
            ret = zcl_sendReportCmd(endpoint, &dstEpInfo,  TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                    claster_id, pAttrEntry->id, pAttrEntry->type, pAttrEntry->data);
        }
    }
}

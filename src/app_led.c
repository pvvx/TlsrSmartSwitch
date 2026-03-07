
#include "app_main.h"

void led_on(void)
{
	gpio_write(dev_gpios.led1,
			(dev_gpios.flg & GPIOS_FLG_LED1_POL)? LED1_OFF : LED1_ON);
}

void led_off(void)
{
	gpio_write(dev_gpios.led1,
			(dev_gpios.flg & GPIOS_FLG_LED1_POL)? LED1_ON : LED1_OFF);
}

bool get_led(void) {
#if LED1_ON
	bool ret = gpio_read(dev_gpios.led1)? 0 : 1;
#else
	bool ret = gpio_read(dev_gpios.led1)? 1 : 0;
#endif
	ret ^= (uint8_t)dev_gpios.flg;
	return ret & GPIOS_FLG_LED1_POL; // return only 0 or 1 !
}

void led_set_control(void) {

    switch(cfg_on_off.led_control) {
        case CONTROL_LED_OFF:
            led_off();
            break;
        case CONTROL_LED_ON:
            led_on();
            break;
        case CONTROL_LED_ON_OFF:
            if (get_relay_status())
            	led_on();
            else
            	led_off();
            break;
        default:
            break;
    }
}

void light_on(void)
{
    if(!g_appCtx.timerLedEvt && cfg_on_off.led_control != CONTROL_LED_OFF)
    	led_on();
}

void light_off(void)
{
    if(!g_appCtx.timerLedEvt && cfg_on_off.led_control != CONTROL_LED_ON)
    	led_off();
}


int32_t zclLightTimerCb(void *arg)
{
    int32_t interval = 0;

    if(g_appCtx.sta == g_appCtx.oriSta) {
        g_appCtx.times--;
        if(g_appCtx.times <= 0){
            g_appCtx.timerLedEvt = NULL;
            led_set_control();
            return -1;
        }
    }
    if(g_appCtx.sta){
    	g_appCtx.sta = 0;
        led_on();
        interval = g_appCtx.ledOnTime;
    } else {
    	g_appCtx.sta = 1;
		led_off();
        interval = g_appCtx.ledOffTime;
    }

    return interval;
}

void light_blink_start(uint8_t times, uint16_t ledOnTime, uint16_t ledOffTime)
{
    uint32_t interval = 0;
    g_appCtx.times = times;

    if(!g_appCtx.timerLedEvt) {
    	g_appCtx.oriSta = get_led();
    	if(g_appCtx.oriSta) { // LED_ON
            g_appCtx.sta = 0;
    		led_off();
            interval = ledOffTime;
        } else {
            g_appCtx.sta = 1;
        	led_on();
            interval = ledOnTime;
        }
        g_appCtx.ledOnTime = ledOnTime;
        g_appCtx.ledOffTime = ledOffTime;

        g_appCtx.timerLedEvt = TL_ZB_TIMER_SCHEDULE(zclLightTimerCb, NULL, interval);
    }
}

void light_blink_stop(void)
{
    if(g_appCtx.timerLedEvt){
        TL_ZB_TIMER_CANCEL(&g_appCtx.timerLedEvt);
        g_appCtx.times = 0;
        led_set_control();
    }
}

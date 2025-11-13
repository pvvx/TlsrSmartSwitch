#include "app_main.h"
#if USE_BL0942
#include "battery.h"

#define HEAD            0x55

typedef struct __attribute__((packed)) {
    uint8_t     head;
    uint32_t    i_rms       :24;
    uint32_t    v_rms       :24;
    uint32_t    i_fast_rms  :24;
    int32_t     watt        :24;
    uint32_t    cf_cnt      :24;
    uint16_t    freq;
    uint8_t     resv1;
    uint8_t     status;
    uint8_t     resv2;
    uint8_t     resv3;
    uint8_t     crc;
} app_monitoring_t;

#define PKT_SIZE    sizeof(app_monitoring_t)

// Address save energy count to Flash
typedef struct {
    uint32_t flash_addr_save;          /* flash page address saved */
    uint32_t flash_addr_start;          /* flash page address start */
    uint32_t flash_addr_end;            /* flash page address end   */
} energy_cons_t;

static energy_cons_t energy_cons;

// Save energy count in Flash
typedef struct {
    uint64_t energy;
    uint64_t xor_energy;
} save_data_t;

static save_data_t save_data;


// UART RX DMA buffer
typedef struct __attribute__((packed)) {
    volatile u32 dma_len;
    u8 data[32];
} dma_uart_rx_buf_t;

dma_uart_rx_buf_t urxb;

// This REF get from https://github.com/esphome/esphome/blob/dev/esphome/components/bl0942/bl0942.h
#define BL0942_POWER_REF        0.596  // x1000: x1000: 0..327.67W, x100 32.767..327.67W, x10: 327.67..3276.7W
#define BL0942_VOLTAGE_REF      159.5 // 158.7335944299 // x100: 0..655.35V
#define BL0942_CURRENT_REF      251.21346469622 // 305978/1.218, x1000: 0..65.535A
#define BL0942_ENERGY_REF       33.0461127328 // x100

static uint8_t  pkt_in[PKT_SIZE];       // frame buffer from BL0942
static uint64_t cur_sum_delivered;      // energy meter
static uint32_t new_energy, old_energy; // for calculating the energy meter
static uint8_t  first_start = true;     // flag
static bool new_save_data = false;      // flag

// Chk BL0942
static uint8_t checksum(uint8_t *data, uint16_t length) {
    uint8_t crc8 = 0;
    for(uint8_t i = 0; i < (length - 1); i++) {
        crc8 += data[i];
    }
    crc8 += 0x58;
    return ~crc8;
}

//-----------------------
// UART 4800 BAUD
#define UART_BAUD 4800 // bps
#if CLOCK_SYS_CLOCK_HZ == 16000000
#define uartCLKdiv 475 // 16000000/(6+1)/(475+1) = 4801.92
#define bwpc 6
#elif CLOCK_SYS_CLOCK_HZ == 24000000
#define uartCLKdiv 999 // 24000000/(4+1)/(999+1) = 4800
#define bwpc 4
#elif CLOCK_SYS_CLOCK_HZ == 32000000
#define uartCLKdiv 832 // 32000000/(7+1)/(832+1) = 4796.163
#define bwpc 7
#elif CLOCK_SYS_CLOCK_HZ == 48000000
#define uartCLKdiv 999 // 48000000/(9+1)/(999+1) = 4800
#define bwpc 9
#endif

// Initializing UART for BL0942
void app_sensor_init(void) {

    drv_uart_pin_set(GPIO_UART_TX, GPIO_UART_RX);

    reg_clk_en0 |= FLD_CLK0_UART_EN; // Enable CLK UART

    // reg_clk_en1 |= FLD_CLK1_DMA_EN;

    uart_reset();

    uart_recbuff_init((unsigned char *)&urxb, sizeof(urxb.data));

    reg_dma0_addr = (uint16_t)((uint32_t)(&urxb)); //set receive buffer address
    reg_dma0_size = sizeof(urxb.data)>>4; //set receive buffer size
    reg_dma0_mode = FLD_DMA_WR_MEM;   //set DMA 0 mode to 0x01 for receive
    reg_dma0_addrHi = 0x04;

    // set reg_uart_clk_div/reg_uart_ctrl0
    REG_ADDR32(0x094) = MASK_VAL(FLD_UART_CLK_DIV, uartCLKdiv, FLD_UART_CLK_DIV_EN, 1)
        | ((MASK_VAL(FLD_UART_BPWC, bwpc) | (FLD_UART_RX_DMA_EN)) << 16) // set bit width, enable UART RX DMA mode
        | ((MASK_VAL(FLD_UART_CTRL1_STOP_BIT, 0)) << 24) // 00: 1 bit, 01: 1.5bit 1x: 2bits;
        ;
    reg_dma_chn_en |= FLD_DMA_CHN_UART_RX;

}

// Clearing USER_DATA
static void clear_user_data(void) {

    uint32_t flash_addr = energy_cons.flash_addr_start;
    while(flash_addr < energy_cons.flash_addr_end) {
        flash_erase_sector(flash_addr);
        flash_addr += FLASH_SECTOR_SIZE;
    }
}


// Saving the energy meter
static void save_dataCb(void *args) {

	battery_detect(0);

    if (save_data.xor_energy != ~save_data.energy) {

        save_data.xor_energy = ~save_data.energy;

        light_blink_start(1, 250, 250);

        if (energy_cons.flash_addr_save == energy_cons.flash_addr_end) {
            energy_cons.flash_addr_save = energy_cons.flash_addr_start;
        }
        if ((energy_cons.flash_addr_save & (FLASH_SECTOR_SIZE-1)) == 0) {
            flash_erase(energy_cons.flash_addr_save);
        }

        flash_write(energy_cons.flash_addr_save, sizeof(save_data), (uint8_t*)&save_data);

        energy_cons.flash_addr_save += sizeof(save_data);
    }
}

// Initializing USER_DATA storage addresses in Flash memory
static void init_save_addr_drv(void) {

	u32 mid = flash_read_mid();
	mid >>= 16;
	mid &= 0xff;
	if(mid >= FLASH_SIZE_1M) {
        energy_cons.flash_addr_start = BEGIN_USER_DATA_F1M;
        energy_cons.flash_addr_end = END_USER_DATA_F1M;
    } else {
        energy_cons.flash_addr_start = BEGIN_USER_DATA_F512K;
        energy_cons.flash_addr_end = END_USER_DATA_F512K;
    }
    energy_cons.flash_addr_save = energy_cons.flash_addr_start;
    save_data.energy = 0;
    save_data.xor_energy = -1;
    g_zcl_seAttrs.cur_sum_delivered = 0;
}


// Task
void monitoring_handler(void) {

    int32_t  power;

    app_monitoring_t *pkt = (app_monitoring_t*)urxb.data;

    if(reg_dma_rx_rdy0 & FLD_DMA_IRQ_UART_RX) { // new data ?
        reg_uart_status0 |= FLD_UART_CLEAR_RX_FLAG | FLD_UART_RX_ERR_FLAG;
        if(urxb.dma_len == PKT_SIZE
          && pkt->head == HEAD
          && checksum((uint8_t *)pkt, PKT_SIZE) == pkt->crc) {
            memcpy(pkt_in, pkt, PKT_SIZE);
            reg_dma_rx_rdy0 = FLD_DMA_IRQ_UART_RX;

            pkt = (app_monitoring_t*)pkt_in;

            g_zcl_msAttrs.current = (uint16_t)((float)(pkt->i_rms/BL0942_CURRENT_REF));

            g_zcl_msAttrs.voltage = (uint16_t)((float)(pkt->v_rms/BL0942_VOLTAGE_REF));

            // power x1000 0..3276.750W
            power = (int32_t)((float)(pkt->watt/BL0942_POWER_REF));

            if(power < 0)
                power =  -power;

            if(power > 327670) {
                // x10: 327.6..3276.7W
                power += 50;
                power /= 100;
                g_zcl_msAttrs.power_divisor = 10;
            } else if(power > 32767) {
                // x100 32.767..327.67W
                power += 5;
                power /= 10;
                g_zcl_msAttrs.power_divisor = 100;
            } else {
                // x1000: x1000: 0..32.767W
                g_zcl_msAttrs.power_divisor = 1000;
            }

            g_zcl_msAttrs.power = (int16_t)power;

            g_zcl_msAttrs.freq = (uint16_t)((float)(100000000.0/pkt->freq));

            new_energy = (uint32_t)((float)(pkt->cf_cnt/BL0942_ENERGY_REF));

            if (first_start) {
                first_start = false;
                old_energy = new_energy;
                return;
            }

            if (new_energy > old_energy) {
                cur_sum_delivered = save_data.energy + (new_energy - old_energy);
                old_energy = new_energy;
                save_data.energy = cur_sum_delivered;
                g_zcl_seAttrs.cur_sum_delivered = cur_sum_delivered;
                new_save_data = true; // energy_save();
            }
        }
    } else {
        reg_dma_rx_rdy0 = FLD_DMA_IRQ_UART_RX;
    }
}

// Read & check valid blk
static int check_saved_blk(uint32_t flash_addr, save_data_t * pdata) {
    if(flash_addr >= energy_cons.flash_addr_end)
        flash_addr = energy_cons.flash_addr_start;
    flash_read_page(flash_addr, sizeof(save_data_t), (uint8_t*)pdata);
    if(pdata->energy == -1 && pdata->xor_energy == -1) {
        return -1;
    } else if(pdata->energy == ~pdata->xor_energy) {
        return 0;
    }
    return 1;
}

// Start initialize
int energy_restore(void) {
    int ret;
    uint32_t flash_addr;

    save_data_t tst_data;

    init_save_addr_drv();

    flash_addr = energy_cons.flash_addr_start;

    while(flash_addr < energy_cons.flash_addr_end) {
        flash_addr &= ~(FLASH_SECTOR_SIZE-1);
        ret = check_saved_blk(flash_addr, &tst_data);
        if(ret < 0) {
            flash_addr += FLASH_SECTOR_SIZE;
            continue;
        }
        if(ret == 0) {
            memcpy(&save_data, &tst_data, sizeof(save_data)); // save_data = tst_data;
            if((check_saved_blk(flash_addr + FLASH_SECTOR_SIZE, &tst_data) == 0)
            && tst_data.energy > save_data.energy) {
                flash_addr += FLASH_SECTOR_SIZE;
                continue;
            }
        }
        flash_addr += sizeof(tst_data);
        while(flash_addr < energy_cons.flash_addr_end) {
            ret = check_saved_blk(flash_addr, &tst_data);
            if(ret == 0) {
                if(tst_data.energy > save_data.energy) {
                    memcpy(&save_data, &tst_data, sizeof(save_data)); // save_data = tst_data;
                } else {
                    flash_addr &= ~(FLASH_SECTOR_SIZE-1);
                    energy_cons.flash_addr_save = flash_addr;
                    g_zcl_seAttrs.cur_sum_delivered = save_data.energy;
                    return 0;
                }
            } else if(ret < 0) {
                energy_cons.flash_addr_save = flash_addr;
                g_zcl_seAttrs.cur_sum_delivered = save_data.energy;
                return 0;
            }
            flash_addr += sizeof(tst_data);
        }
    }
    return 1;
}

// Step 1 minutes
int32_t energy_timerCb(void *args) {

    if (new_save_data) {
        new_save_data = false;
        TL_SCHEDULE_TASK(save_dataCb, NULL);
    }
    return 0;
}

// Step 1 sec
int32_t app_monitoringCb(void *arg) {

    REG_ADDR16(0x90) = 0xAA58; // Send cmd: "Read full packet"

    return 0;
}

// Clear all USER_DATA
void energy_remove(void) {
    init_save_addr_drv();
    clear_user_data();
}

#endif // USE_BL0942

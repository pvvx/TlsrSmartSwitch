/*
 * energy_save.c
 *
 *  Created on: 13 нояб. 2025 г.
 *      Author: pvvx
 *
 * Максимальное значение energy за час 25A*250V = 6250Wh
 * 6250/60 = 104.166666667 W в минуту.
 * число 10416 - (в единицах 0.01Вт) влезает в uint16_t c запасом в 5 раз.
 * Если писать 8 байт uint64_t energy + uint32_t crc32,
 * и далее uint16_t delta + uint16_t chk,
 * тогда в секторе выйдет (4096-8-4)/4 = 1021 запись + 1 в заголовке сектора.
 * За 1022/60 = 17.03333 часов заполнится один сектор.
 * 4 сектора для Flash 512K заполнится за 4*1022/60 = 68.1333 часов,
 * т.е. за 4*1022/60/24 = 2.83889 суток.
 * Тогда до исчерпания ресурса записи Flash в 10k для 4-х секторов получим:
 * 10000*4*1022/60/24 = 28388.889 суток, или 28388.889/365 = 77.777 лет
 * Для Flash c 1M и буфером записей в 102 сектора ресурс составит:
 * примерно 77.777*102/4 = 1983.3135 лет
 * Но типчно SPI-Flash выдерживает более 20 тысяч перезаписей...
 */

#include "app_main.h"
#ifdef ZCL_METERING
#include "energy_save.h"
#include "battery.h"
#include "utility.h"

energy_store_head_t save_data;

// Store delta
typedef struct {
	uint16_t delta;
	uint16_t chk;
} energy_store_delta_t;

// Address save energy count to Flash
typedef struct {
    uint32_t flash_addr_save;          /* flash page address saved */
    uint32_t flash_addr_start;         /* flash page address start */
    uint32_t flash_addr_end;           /* flash page address end   */
} energy_cons_t;

static energy_cons_t energy_cons; // Address save energy count to Flash

bool new_save_data = false;      // flag

// Clearing USER_DATA
static void clear_user_data(void) {
    uint32_t flash_addr = energy_cons.flash_addr_start;
    while(flash_addr < energy_cons.flash_addr_end) {
        flash_erase_sector(flash_addr);
        flash_addr += FLASH_SECTOR_SIZE;
    }
}

// Saving the energy meter
//__attribute__((optimize("-Os")))
static void save_dataCb(void *args) {

	uint32_t sector_count;
	energy_store_delta_t energy_delta;
    energy_store_head_t tst_head;

	battery_detect(0);

	energy_delta.delta = g_zcl_seAttrs.cur_sum_delivered - save_data.energy;
	// if < 0.1/60 Wh (всё отключено) -> пропустить эту запись,
	// будет записано в следующий раз, когда накопится больше
	if(energy_delta.delta > 10) {
		save_data.energy = g_zcl_seAttrs.cur_sum_delivered;
	    if (energy_cons.flash_addr_save >= energy_cons.flash_addr_end) {
	        energy_cons.flash_addr_save = energy_cons.flash_addr_start;
	    }
	    if ((energy_cons.flash_addr_save & (FLASH_SECTOR_SIZE-1)) == 0) {
    		save_data.crc = xcrc32((uint8_t*)&save_data.energy, sizeof(save_data.energy), 0xffffffff);
    		// запись заголовка сектора с проверкой
	    	sector_count = (energy_cons.flash_addr_end - energy_cons.flash_addr_start)/FLASH_SECTOR_SIZE;
    		while(sector_count--) {
    			flash_erase_sector(energy_cons.flash_addr_save);
	    		flash_write_page(energy_cons.flash_addr_save, sizeof(save_data), (uint8_t*)&save_data);
	    	    flash_read_page(energy_cons.flash_addr_save, sizeof(tst_head), (uint8_t*)&tst_head);
	    		if(memcmp(&save_data, &tst_head, sizeof(save_data)) == 0)
	    			break;
    			energy_cons.flash_addr_save += FLASH_SECTOR_SIZE;
	    	    if (energy_cons.flash_addr_save >= energy_cons.flash_addr_end) {
	    	        energy_cons.flash_addr_save = energy_cons.flash_addr_start;
	    	    }
	    	}
	    	energy_cons.flash_addr_save += sizeof(save_data);
	    } else {
	    	// запись дельты
	    	energy_delta.chk = ~energy_delta.delta;
	    	flash_write_page(energy_cons.flash_addr_save, sizeof(energy_delta), (uint8_t*)&energy_delta);
	    	energy_cons.flash_addr_save += sizeof(energy_delta);
	    }
	}
}

// Initializing USER_DATA storage addresses in Flash memory
static void init_save_addr_drv(void) {

	u32 mid = flash_read_mid();
	mid >>= 16;
	mid &= 0xff;
	if(mid == FLASH_SIZE_1M) {
        energy_cons.flash_addr_start = BEGIN_USER_DATA_F1M;
        energy_cons.flash_addr_end = END_USER_DATA_F1M;
    } else {
        energy_cons.flash_addr_start = BEGIN_USER_DATA_F512K;
        energy_cons.flash_addr_end = END_USER_DATA_F512K;
    }
    energy_cons.flash_addr_save = energy_cons.flash_addr_start;
    save_data.energy = 0;
    g_zcl_seAttrs.cur_sum_delivered = 0;
}

// Read & check valid head (Save Energy)
static int check_saved_head(uint32_t flash_addr, energy_store_head_t * pdata) {
    if(flash_addr >= energy_cons.flash_addr_end)
        flash_addr = energy_cons.flash_addr_start;
    flash_read_page(flash_addr, sizeof(energy_store_head_t), (uint8_t*)pdata);
    if(pdata->energy == (uint64_t)0xffffffffffffffff && pdata->crc == 0xffffffff) {
        return -1;
    } else if(pdata->crc == xcrc32((uint8_t*)pdata, sizeof(uint64_t), 0xffffffff)) {
        return 0;
    }
    return 1;
}

// Read & check valid delta (Save Energy)
static int check_saved_delta(uint32_t flash_addr, energy_store_delta_t * pdata) {
    if(flash_addr >= energy_cons.flash_addr_end)
        flash_addr = energy_cons.flash_addr_start;
    flash_read_page(flash_addr, sizeof(energy_store_delta_t), (uint8_t*)pdata);
    if(pdata->delta == 0xffff && pdata->chk == 0xffff) {
        return -1;
    } else if((pdata->delta ^ pdata->chk) == 0xffff) {
        return 0;
    }
    return 1;
}

// Start initialize (Save Energy)
//__attribute__((optimize("-Os")))
void energy_restore(void) {
    int ret;
    uint32_t flash_addr;

    energy_store_head_t tst_head;
    energy_store_delta_t tst_delta;

    init_save_addr_drv();

    flash_addr = energy_cons.flash_addr_start;

    while(flash_addr < energy_cons.flash_addr_end) {
        // начинаем всегда с начала сектора, с заголовка
    	flash_addr &= ~(FLASH_SECTOR_SIZE-1);
    	// проверка заголовка сектора и получение начального счетчика
        ret = check_saved_head(flash_addr, &tst_head);
        if(ret < 0) {
        	// сектор пустой, cмотрим следующий
            flash_addr += FLASH_SECTOR_SIZE;
            continue;
        } else if(ret == 0) {
        	// сектор с верным заголовком, сохраним energy
        	save_data.energy = tst_head.energy;
            // проверим следующий сектор (с учетом перехода на начало области сохранений)
            ret = check_saved_head(flash_addr + FLASH_SECTOR_SIZE, &tst_head);
            if(ret == 0 && (tst_head.energy >= save_data.energy)) {
            	// сектор с верным заголовком
            	// значение energy больше, переходим на следующий сектор.
                flash_addr += FLASH_SECTOR_SIZE;
                continue;
            }
        	// тут: следующий сектор пуст или следующий сектор с битым заголовком
            // flash_addr = предыдущему сектору с найденным верным заголовком
            // Будем исследовать дельты ранее найденного сектора
        } else if(ret > 0) {
        	// сектор с битым заголовком
        	// Это говорит о том, что при записи заголовка отключили питание
        	// и сектор только начат для записи (или во Flash бардак)
        	// Но, если это первый сектор, тогда возможна потеря данных
        	// - следует проверить следущий
        	if(flash_addr == energy_cons.flash_addr_start) {
                flash_addr += FLASH_SECTOR_SIZE;
                continue;
        	} else {
        		// сектор с битым заголовком, но не первый
        		flash_erase_sector(flash_addr);
        		g_zcl_seAttrs.cur_sum_delivered = save_data.energy;
        		energy_cons.flash_addr_save = flash_addr;
        		return;
        	}
        }
        // flash_addr = последний найденный сектор с заголовком
        flash_addr += sizeof(tst_head);
        // исследуем и суммируем записи c delta
        while((flash_addr & (FLASH_SECTOR_SIZE-1)) != 0) {
        	ret = check_saved_delta(flash_addr, &tst_delta);
            if(ret == 0) {
            	// chk delta = ok, добавим дельту
            	save_data.energy += (uint64_t)tst_delta.delta;
            } else if(ret < 0) {
            	// пустой блок, смотрим следующий, т.е. ожидаем 2 пустых:
            	ret = check_saved_delta(flash_addr + sizeof(tst_delta), &tst_delta);
            	if(ret < 0) { // тоже пустой -> найден конец записей
            				  // даже если это конец сектора - этот сектор будет откинут
            		g_zcl_seAttrs.cur_sum_delivered = save_data.energy;
            		energy_cons.flash_addr_save = flash_addr;
            		return;
            	}
#if 0	// Избыточно - лучше пропустить все непрописанные/невалидные значения,
        // если при их записи был срыв питания.
        // И так меньше объем кода, а для 512K Flash это актуально
        	} else {
            	// запись bad, смотрим следующий ?
        		ret = check_saved_delta(flash_addr + sizeof(tst_delta), &tst_delta);
            	if(ret > 0) {
            		// следующая запись снова bad:
            		// начнем запись со следюущего сектора
            		flash_addr += FLASH_SECTOR_SIZE-1;
            		flash_addr &= ~(FLASH_SECTOR_SIZE-1);
            		g_zcl_seAttrs.cur_sum_delivered = save_data.energy;
            		energy_cons.flash_addr_save = flash_addr;
            		return;
            	}
            	// следующая запись = ok или пусто, продолжим
#endif
            }
            flash_addr += sizeof(tst_delta);
        }
        // следующий сектор
		g_zcl_seAttrs.cur_sum_delivered = save_data.energy;
        energy_cons.flash_addr_save = flash_addr;
        return;
    }
	// выход while:  нет записей - все сектора пусты:
	// flash_addr = energy_cons.flash_addr_end
	g_zcl_seAttrs.cur_sum_delivered = save_data.energy;
    energy_cons.flash_addr_save = flash_addr;
    return;
}

// Clear all USER_DATA (Save Energy)
void energy_remove(void) {
    init_save_addr_drv();
    clear_user_data();
}

// Step 1 minutes (Save Energy)
int32_t energy_timerCb(void *args) {

    if (new_save_data) {
        new_save_data = false;
        TL_SCHEDULE_TASK(save_dataCb, NULL);
    }
    return 0;
}
#endif // ZCL_METERING

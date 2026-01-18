/*
 * energy_save.h
 *
 *  Created on: 13 нояб. 2025 г.
 *      Author: pvvx
 */

#ifndef SRC_ENERGY_SAVE_H_
#define SRC_ENERGY_SAVE_H_

#ifdef ZCL_METERING
// Head, start sector Flash
typedef struct {
	uint64_t energy; // Save energy count in Flash
	uint32_t crc;
} energy_store_head_t;
extern energy_store_head_t save_data;
extern bool new_save_data;    // flag

void energy_restore(void); // Start initialize (Save Energy)
void energy_remove(void); // Clear all USER_DATA (Save Energy)
int32_t energy_timerCb(void *args); // Step 1 minutes (Save Energy)

#endif // ZCL_METERING

#endif /* SRC_ENERGY_SAVE_H_ */

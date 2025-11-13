#ifndef SRC_INCLUDE_APP_SENSOR_H_
#define SRC_INCLUDE_APP_SENSOR_H_

int32_t app_monitoringCb(void *arg);
int32_t energy_timerCb(void *args);
void monitoring_handler(void);
int energy_restore(void);
void energy_remove(void);

void app_sensor_init(void);

#endif /* SRC_INCLUDE_APP_SENSOR_H_ */

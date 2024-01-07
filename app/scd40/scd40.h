#ifndef SCD40_H
#define SCD40_H

#include <stdbool.h>
#include <stdint.h>

#define SCD40_ADDRESS 0x62

int16_t scd40_init();
int16_t scd40_read_measurement(uint16_t *co2, int32_t *temp_mc,
                               int16_t *humidity_m_percent_rh);
int16_t scd40_get_data_ready_flag(bool *data_ready_flag);

#define CMD_START_PERIODIC_MEASUREMENT 0x21b1  // - duration
#define CMD_READ_MEASUREMENT 0xec05            // 1ms duration
#define CMD_STOP_PRIODIC_MEASUREMENT 0x3f86    // 500ms duration
#define CMD_SET_SENSOR_ALTITUDE 0x2427         // 1ms duration
#define CMD_GET_DATA_READY_STATUS 0xe4b8       // 1ms duation
#define CMD_PERSIST_SETTINGS 0x3615            // 800ms duration
#define CMD_REINIT 0x3646                      // 30ms duration

#endif  // !SCD40_H

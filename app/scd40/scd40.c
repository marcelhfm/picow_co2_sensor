/*
 * Copyright (c) 2021, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "scd40.h"

#include <FreeRTOS.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <task.h>

#include "../i2c/i2c.h"
#include "pico/stdlib.h"

uint16_t common_bytes_to_uint16_t(const uint8_t *bytes) {
  return (uint16_t)bytes[0] << 8 | (uint16_t)bytes[1];
}

// Internal methods

int16_t scd4x_read_measurement_ticks(uint16_t *co2, uint16_t *temperature,
                                     uint16_t *humidity) {
  int16_t retval;
  uint8_t buffer[9];
  uint16_t offset = 0;

  offset = i2c_add_command_to_buffer(&buffer[0], offset, CMD_READ_MEASUREMENT);

  retval = i2c_write(I2C_PORT, SCD40_ADDRESS, &buffer[0], offset, false);
  if (retval != 0) {
    return retval;
  }

  vTaskDelay(1 / portTICK_PERIOD_MS);

  retval = i2c_read_data_inplace(SCD40_ADDRESS, &buffer[0], 6);
  if (retval != 0) {
    return retval;
  }
  *co2 = common_bytes_to_uint16_t(&buffer[0]);
  *temperature = common_bytes_to_uint16_t(&buffer[2]);
  *humidity = common_bytes_to_uint16_t(&buffer[4]);

  return 0;
}

int16_t start_periodic_measurement() {
  int16_t retval;
  uint8_t buffer[2];
  uint16_t offset = 0;

  offset = i2c_add_command_to_buffer(&buffer[0], offset,
                                     CMD_START_PERIODIC_MEASUREMENT);

  retval = i2c_write(I2C_PORT, SCD40_ADDRESS, &buffer[0], offset, false);
  if (retval != 0) {
    return retval;
  }
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  return 0;
}

int16_t stop_periodic_measurement() {
  int16_t retval;
  uint8_t buffer[2];
  uint16_t offset = 0;
  offset = i2c_add_command_to_buffer(&buffer[0], offset,
                                     CMD_STOP_PRIODIC_MEASUREMENT);

  retval = i2c_write(I2C_PORT, SCD40_ADDRESS, &buffer[0], offset, false);

  if (retval != 0) {
    return retval;
  }
  vTaskDelay(500 / portTICK_PERIOD_MS);
  return 0;
}

int16_t set_altitude(uint16_t altitude) {
  int16_t retval;
  uint8_t buffer[5];
  uint16_t offset = 0;

  offset =
      i2c_add_command_to_buffer(&buffer[0], offset, CMD_SET_SENSOR_ALTITUDE);

  offset = i2c_add_uint16_to_buffer(&buffer[0], offset, altitude);

  retval = i2c_write(I2C_PORT, SCD40_ADDRESS, &buffer[0], offset, false);

  if (retval != 0) {
    return retval;
  }
  vTaskDelay(1 / portTICK_PERIOD_MS);
  return 0;
}

int16_t reinit() {
  int16_t retval;
  uint8_t buffer[2];
  uint16_t offset = 0;

  offset = i2c_add_command_to_buffer(&buffer[0], offset, CMD_REINIT);

  retval = i2c_write(I2C_PORT, SCD40_ADDRESS, &buffer[0], offset, false);
  if (retval != 0) {
    return retval;
  }

  vTaskDelay(30 / portTICK_PERIOD_MS);
  return 0;
}

// Interface methods
int16_t scd40_init() {
  int16_t retval;
  retval = stop_periodic_measurement();

  if (retval != 0) {
    printf("scd40_init: error stopping periodic measurement: %i\n", retval);
    return retval;
  }

  retval = reinit();
  if (retval != 0) {
    printf("scd40_init: error reinitializing sensor: %i\n", retval);
    return retval;
  }

  retval = set_altitude(482);  // meters

  if (retval != 0) {
    printf("scd40_init: error setting altitude: %i\n", retval);
    return retval;
  }

  retval = start_periodic_measurement();
  if (retval != 0) {
    printf("scd40_init: error starting periodic measurement: %i\n", retval);
    return retval;
  }

  return 0;
}

int16_t scd40_read_measurement(uint16_t *co2, int32_t *temp_mc,
                               int16_t *humidity_m_percent_rh) {
  int16_t retval;
  uint16_t temperature;
  uint16_t humidity;

  retval = scd4x_read_measurement_ticks(co2, &temperature, &humidity);
  if (retval != 0) {
    return retval;
  }

  *temp_mc = ((21875 * (int32_t)temperature) << 13) - 45000;
  *humidity_m_percent_rh = ((12500 * (int32_t)humidity) >> 13);
  return 0;
}

int16_t scd40_get_data_ready_flag(bool *data_ready_flag) {
  int16_t retval;
  uint8_t buffer[3];
  uint16_t offset = 0;
  uint16_t local_data_ready = 0;

  offset =
      i2c_add_command_to_buffer(&buffer[0], offset, CMD_GET_DATA_READY_STATUS);

  retval = i2c_write(I2C_PORT, SCD40_ADDRESS, &buffer[0], offset, false);
  if (retval != 0) {
    return retval;
  }

  vTaskDelay(1 / portTICK_PERIOD_MS);
  retval = i2c_read_data_inplace(SCD40_ADDRESS, &buffer[0], 2);
  if (retval != 0) {
    return retval;
  }

  local_data_ready = common_bytes_to_uint16_t(&buffer[0]);

  *data_ready_flag = (local_data_ready & 0x07FF) != 0;
  return 0;
}

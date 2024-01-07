#ifndef I2C_H
#define I2C_H

#include <stdint.h>

#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define WORD_SIZE 2
#define CRC8_POLYNOMIAL 0x31
#define CRC8_INIT 0xFF
#define CRC8_LEN 1

int16_t i2c_read(i2c_inst_t *i2c, uint8_t addr, uint8_t *dst, size_t len,
                 bool nostop);
int16_t i2c_write(i2c_inst_t *i2c, uint8_t addr, uint8_t *src, size_t len,
                  bool nostop);
void init_i2c();
void i2c_scan();
uint8_t i2c_generate_crc(const uint8_t *data, uint16_t count);
uint16_t i2c_add_command_to_buffer(uint8_t *buffer, uint16_t offset,
                                   uint16_t command);
uint16_t i2c_add_uint16_to_buffer(uint8_t *buffer, uint16_t offset,
                                  uint16_t data);
int16_t i2c_read_data_inplace(uint8_t address, uint8_t *buffer,
                              uint16_t expected_data_length);
#endif  // I2C_H

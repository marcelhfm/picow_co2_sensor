#include "i2c.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

#define I2C_PORT i2c0

#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 1

int16_t i2c_write(i2c_inst_t *i2c, uint8_t addr, uint8_t *src, size_t len,
                  bool nostop) {
  int retval = i2c_write_blocking(i2c, addr, src, len, nostop);

  if (retval == PICO_ERROR_GENERIC) {
    printf("i2c_write: Error sending command to i2c device. \n");
    return -1;
  } else if (retval < 0) {
    printf("i2c_write: I2C transaction error occurred: %d\n", retval);
    return -1;
  } else if (retval != len) {
    printf(
        "i2c_write: Mismath in the number of bytes send. Sent: %d, Expected: "
        "%zu",
        retval, len);
    return -1;
  }

  return 0;
}

int16_t i2c_read(i2c_inst_t *i2c, uint8_t addr, uint8_t *dst, size_t len,
                 bool nostop) {
  int retval = i2c_read_blocking(i2c, addr, dst, len, nostop);

  if (retval == PICO_ERROR_GENERIC) {
    printf("i2c_read: Error sending command to i2c device. \n");
    return -1;
  } else if (retval < 0) {
    printf("i2c_read: I2C transaction error occurred: %d\n", retval);
    return -1;
  } else if (retval != len) {
    printf(
        "i2c_read: Mismath in the number of bytes received. Sent: %d, "
        "Expected: "
        "%zu",
        retval, len);
    return -1;
  }

  return 0;
}

void init_i2c() {
  printf("init_i2c: Initializing i2c...\n");
  i2c_init(I2C_PORT, 100 * 1000);  // 100 kHz

  // SSD1306
  gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

  i2c_set_slave_mode(I2C_PORT, false, 0);
  printf("init_i2c: Done\n");
}

void i2c_scan() {
  printf("Scanning I2C bus...\n");
  uint8_t address;
  uint8_t data;

  for (address = 1; address < 128;
       address++) {  // 7-bit addresses from 0x01 to 0x7F
    int result = i2c_write_blocking(I2C_PORT, address, &data, 1, false);

    if (result == 1) {
      printf("I2C device detected at address 0x%02X Result: %d\n", address,
             result);
    }
  }
}

uint8_t i2c_generate_crc(const uint8_t *data, uint16_t count) {
  uint16_t current_byte;
  uint8_t crc = CRC8_INIT;
  uint8_t crc_bit;

  /* calculates 8-Bit checksum with given polynomial */
  for (current_byte = 0; current_byte < count; ++current_byte) {
    crc ^= (data[current_byte]);
    for (crc_bit = 8; crc_bit > 0; --crc_bit) {
      if (crc & 0x80)
        crc = (crc << 1) ^ CRC8_POLYNOMIAL;
      else
        crc = (crc << 1);
    }
  }
  return crc;
}

int8_t i2c_check_crc(const uint8_t *data, uint16_t count, uint8_t checksum) {
  if (i2c_generate_crc(data, count) != checksum) return -1;
  return 0;
}

uint16_t i2c_add_command_to_buffer(uint8_t *buffer, uint16_t offset,
                                   uint16_t command) {
  buffer[offset++] = (uint8_t)((command & 0xFF00) >> 8);
  buffer[offset++] = (uint8_t)((command & 0x00FF) >> 0);
  return offset;
}

uint16_t i2c_add_uint16_to_buffer(uint8_t *buffer, uint16_t offset,
                                  uint16_t data) {
  buffer[offset++] = (uint8_t)((data & 0xFF00) >> 8);
  buffer[offset++] = (uint8_t)((data & 0x00FF) >> 0);
  buffer[offset] = i2c_generate_crc(&buffer[offset - WORD_SIZE], WORD_SIZE);
  offset++;

  return offset;
}

int16_t i2c_read_data_inplace(uint8_t address, uint8_t *buffer,
                              uint16_t expected_data_length) {
  int16_t retval;
  uint16_t i, j;
  uint16_t size = (expected_data_length / WORD_SIZE) * (WORD_SIZE + CRC8_LEN);

  if (expected_data_length % WORD_SIZE != 0) {
    return -1;
  }

  retval = i2c_read(I2C_PORT, address, buffer, size, false);
  if (retval != 0) {
    return retval;
  }

  for (i = 0, j = 0; i < size; i += WORD_SIZE + CRC8_LEN) {
    retval = i2c_check_crc(&buffer[i], WORD_SIZE, buffer[i + WORD_SIZE]);
    if (retval != 0) {
      return retval;
    }
    buffer[j++] = buffer[i];
    buffer[j++] = buffer[i + 1];
  }

  return 0;
}

#include "ssd1306.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../i2c/i2c.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

#define OLED_ADDRESS 0x3C

void clear_screen();

void oled_command(uint8_t command) {
  uint8_t data[] = {0x00, command};
  int retval = i2c_write_blocking(I2C_PORT, OLED_ADDRESS, data, 2, false);

  if (retval == PICO_ERROR_GENERIC) {
    printf("oled_commnad: Error sending command to OLED display.\n");
  } else if (retval < 0) {
    printf("oled_command: I2C transaction error occurred: %d\n", retval);
  } else if (retval != 2) {
    printf(
        "oled_command: Mismatch in the number of bytes send. Sent: %d, "
        "Expected: %zu\n",
        retval, 2);
  }
}

void flash_display(bool invert) {
  if (invert) {
    oled_command(CMD_INVERSE_DISPLAY);
  } else {
    oled_command(CMD_NORMAL_DISPLAY);
  }
}

void oled_init() {
  printf("oled_init: Initializing display...\n");
  oled_command(CMD_DISPLAY_OFF);
  oled_command(CMD_SET_LOW_COLUMN);
  oled_command(CMD_SET_HIGH_COLUMN);
  oled_command(CMD_SET_START_LINE);
  oled_command(CMD_MEMORY_MODE);
  oled_command(CMD_MEMORY_MODE_HORIZONTAL);
  oled_command(CMD_SET_CONTRAST);
  oled_command(0xFF);
  oled_command(CMD_NORMAL_DISPLAY);
  oled_command(CMD_SET_MULTIPLEX);
  oled_command(0x3F);  // 1/64 duty (63)
  oled_command(CMD_SET_DISPLAY_OFFSET);
  oled_command(0x00);  // no offset
  oled_command(CMD_SET_DISPLAY_CLOCK_DIV);
  oled_command(0x80);  // the suggested ratio 0x80
  oled_command(CMD_SET_PRECHARGE);
  oled_command(0x22);
  oled_command(CMD_SET_COM_PINS);
  oled_command(0x12);
  oled_command(CMD_SET_VCOM_DESELECT);
  oled_command(0x40);  // lowest / dimmest
  oled_command(CMD_CHARGE_PUMP);
  oled_command(0x14);  // Enable charge pump
  oled_command(CMD_DISPLAY_ALL_ON_RESUME);
  oled_command(CMD_DISPLAY_ON);

  printf("oled_init: Done\n");
}

void set_page_start(uint8_t start) { oled_command(0xB0 | (start & 0x07)); }

void set_column(uint8_t col) {
  oled_command(0x00 | ((col >> 0) & 0x0F));
  oled_command(0x10 | ((col >> 4) & 0x0F));
}

void send_data(const uint8_t* data, size_t length) {
  uint8_t data_with_control[length + 1];
  data_with_control[0] = 0x40;
  memcpy(data_with_control + 1, data, length);
  int retval = i2c_write_blocking(I2C_PORT, OLED_ADDRESS, data_with_control,
                                  length + 1, false);

  if (retval == PICO_ERROR_GENERIC) {
    printf("send_data: Error sending data to OLED display.\n");
  } else if (retval < 0) {
    printf("send_data: I2C transaction error occurred: %d\n", retval);
  } else if (retval != length + 1) {
    printf(
        "send_data: Mismatch in the number of bytes sent. Sent: %d, Expected: "
        "%zu\n",
        retval, length + 1);
  }
}

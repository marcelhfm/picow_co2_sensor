#include "ssd1306.h"

#include <stdio.h>
#include <string.h>

#include "font8x8_basic.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 1

#define OLED_ADDRESS 0x3C

void clear_screen();

void init_i2c() {
  printf("init_i2c: Initializing i2c...\n");
  i2c_init(I2C_PORT, 100 * 1000);  // 100 kHz
  gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
  i2c_set_slave_mode(I2C_PORT, false, NULL);
  printf("init_i2c: Done\n");
}

void i2c_scan() {
  printf("Scanning I2C bus...\n");
  uint8_t address;
  uint8_t data;

  for (address = 1; address < 128;
       address++) {  // 7-bit addresses from 0x01 to 0x7F
    int result = i2c_read_blocking(I2C_PORT, address, &data, 1, false);

    if (result !=
        PICO_ERROR_GENERIC) {  // Check if there is a response from the address
      printf("I2C device detected at address 0x%02X\n", address);
    }
  }
}

void oled_command(uint8_t command) {
  uint8_t data[] = {0x00, command};
  i2c_write_blocking(I2C_PORT, OLED_ADDRESS, data, 2, false);
  // printf("Command sent: 0x%02X\n", command);
}

void flash_display(uint8_t invert) {
  if (invert == 1) {
    oled_command(CMD_INVERSE_DISPLAY);
  } else {
    oled_command(CMD_NORMAL_DISPLAY);
  }
}

void oled_init() {
  printf("oled_init: Initializing display...\n");
  oled_command(CMD_DISPLAY_OFF);
  oled_command(CMD_SET_DISPLAY_CLOCK_DIV);
  oled_command(0x80);  // the suggested ratio 0x80
  oled_command(CMD_SET_MULTIPLEX);
  oled_command(0x3F);  // 1/64 duty (63)
  oled_command(CMD_SET_DISPLAY_OFFSET);
  oled_command(0x00);                      // no offset
  oled_command(CMD_SET_START_LINE | 0x0);  // Line #0)
  oled_command(CMD_CHARGE_PUMP);
  oled_command(0x14);  // Enable charge pump
  oled_command(CMD_MEMORY_MODE);
  oled_command(0x20);
  oled_command(CMD_SEG_REMAP | 0x1);
  oled_command(CMD_COM_SCAN_DEC);
  oled_command(CMD_SET_COM_PINS);
  oled_command(0x12);
  oled_command(CMD_SET_CONTRAST);
  oled_command(0x0);  // dimmest
  oled_command(CMD_SET_PRECHARGE);
  oled_command(0xF1);
  oled_command(CMD_SET_VCIM_DESELECT);
  oled_command(0x00);  // lowest / dimmest
  oled_command(CMD_NORMAL_DISPLAY);
  oled_command(CMD_DISPLAY_ALL_ON_RESUME);
  clear_screen();
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
  i2c_write_blocking(I2C_PORT, OLED_ADDRESS, data_with_control, length + 1,
                     false);
}

void clear_screen() {
  uint8_t zero[] = {0x00};

  for (uint8_t page = 0; page < 8; page++) {
    set_page_start(page);
    for (uint8_t col = 0; col < 128; col++) {
      set_column(col);
      send_data(zero, 1);
    }
  }
}

void oled_string(char* message) {
  while (*message) {
    const uint8_t* char_pattern = font8x8_basic[(uint8_t)*message];
    send_data(char_pattern, 8);

    uint8_t padding[] = {0x00};
    send_data(padding, 1);

    message++;
  }
}
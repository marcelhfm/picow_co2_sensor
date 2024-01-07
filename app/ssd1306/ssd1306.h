#ifndef ssd1306_h
#define ssd1306_h

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "pico/stdio.h"

// Commands
#define CMD_SET_CONTRAST 0x81
#define CMD_DISPLAY_ALL_ON_RESUME 0xA4
#define CMD_DISPLAY_ALL_ON 0xA5
#define CMD_NORMAL_DISPLAY 0xA6
#define CMD_INVERSE_DISPLAY 0xA7
#define CMD_DISPLAY_OFF 0xAE
#define CMD_DISPLAY_ON 0xAF
#define CMD_SET_DISPLAY_OFFSET 0xD3
#define CMD_SET_COM_PINS 0xDA
#define CMD_SET_VCOM_DESELECT 0xDB
#define CMD_SET_DISPLAY_CLOCK_DIV 0xD5
#define CMD_SET_PRECHARGE 0xD9
#define CMD_SET_MULTIPLEX 0xA8
#define CMD_SET_LOW_COLUMN 0x00
#define CMD_SET_HIGH_COLUMN 0x10
#define CMD_SET_START_LINE 0x40
#define CMD_MEMORY_MODE 0x20
#define CMD_MEMORY_MODE_HORIZONTAL 0x00
#define CMD_MEMORY_MODE_VERTICAL 0x01
#define CMD_MEMORY_MODE_PAGE 0x10
#define CMD_COM_SCAN_INC 0xC0
#define CMD_COM_SCAN_DEC 0xC8
#define CMD_SEG_REMAP 0xA0
#define CMD_CHARGE_PUMP 0x8D
#define CMD_EXTERNAL_VCC 0x01
#define CMD_SWITCH_CAP_VCC 0x02
#define CMD_PAGE_ADDR 0x22
#define CMD_COL_ADDR 0x21
#define CMD_START_LINE 0x40
#define CMD_COL_REMAP_OFF 0xC0
#define CMD_COL_REMAP_ON 0xC8
#define CMD_COM_REMAP_OFF 0xA0
#define CMD_COM_REMAP_ON 0xA1

void oled_command(uint8_t command);
void oled_init();
void oled_string(char* message);
void flash_display(bool invert);
void clear_screen();
void send_data(const uint8_t* data, size_t length);
#endif

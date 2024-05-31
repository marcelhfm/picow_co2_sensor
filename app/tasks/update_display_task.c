#include "update_display_task.h"

#include <FreeRTOS.h>
#include <queue.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../ssd1306/display.h"
#include "../ssd1306/framebuffer.h"
#include "../ssd1306/ssd1306.h"
#include "cyw43.h"
#include "portmacro.h"

extern QueueHandle_t display_queue;

void draw_checkmark(FrameBuffer *fb, int x, int y, enum WriteMode wm) {
  display_set_pixel(fb, x + 0, y + 5, wm);
  display_set_pixel(fb, x + 1, y + 6, wm);
  display_set_pixel(fb, x + 2, y + 7, wm);
  display_set_pixel(fb, x + 3, y + 6, wm);
  display_set_pixel(fb, x + 4, y + 5, wm);
  display_set_pixel(fb, x + 5, y + 4, wm);
  display_set_pixel(fb, x + 6, y + 3, wm);
  display_set_pixel(fb, x + 7, y + 2, wm);
}

void draw_cross(FrameBuffer *fb, int x, int y, enum WriteMode wm) {
  for (int i = 0; i < 8; ++i) {
    display_set_pixel(fb, x + i, y + i, wm);
    display_set_pixel(fb, x + 7 - i, y + i, wm);
  }
}

void update_display(DisplayInfo *display_info, FrameBuffer *fb,
                    enum WriteMode wm, enum Rotation rot) {
  fb_clear(fb);

  // Draw status bar
  display_draw_text(fb, "S:", 0, 0, wm, rot);
  display_draw_text(fb, "W:", 128 - 8 - 8 - 8, 0, wm, rot);

  if (display_info->sensor_status == STATUS_GOOD) {
    draw_checkmark(fb, 16, 0, wm);
  } else if (display_info->sensor_status == STATUS_BAD) {
    draw_cross(fb, 16, 0, wm);
  }

  if (display_info->wifi_status == STATUS_GOOD) {
    draw_checkmark(fb, 120, 0, wm);
  } else if (display_info->wifi_status == STATUS_BAD) {
    draw_cross(fb, 120, 0, wm);
  }

  for (int i = 0; i < 128; i++) {
    display_set_pixel(fb, i, 10, wm);
  }

  // Draw co2 levels
  char str[12];
  sprintf(str, "%d", display_info->co2_measurement);
  strcat(str, " ppm");

  display_draw_text(fb, str, 128 / 3, 20, wm, rot);
  // Draw co2 levels bar + warning
  // Calculate the fill level based on co2_measurement (range: 0 - 2500)
  uint8_t fillLevel =
      (uint8_t)((display_info->co2_measurement * (128 - 15 - 14)) / 2500);

  // Adjust fill level if it exceeds the bar's width
  if (fillLevel > (128 - 15 - 14)) {
    fillLevel = (128 - 15 - 14);
  }

  // Draw CO2 levels bar
  // Bar outline
  for (uint8_t i = 15; i < 128 - 14; i++) {
    display_set_pixel(fb, i, 32, wm);
    display_set_pixel(fb, i, 36, wm);
  }
  for (uint8_t i = 32; i <= 36; i++) {
    display_set_pixel(fb, 15, i, wm);
    display_set_pixel(fb, 128 - 14, i, wm);
  }

  // Fill in the bar based on the CO2 measurement
  for (uint8_t i = 16; i < 16 + fillLevel; i++) {
    for (uint8_t j = 33; j <= 35; j++) {
      display_set_pixel(fb, i, j, wm);
    }
  }

  // Status text
  if (display_info->co2_measurement < 400) {
    display_draw_text(fb, "Very Good", 128 / 3 - 15, 42, wm, rot);
  } else if (display_info->co2_measurement > 2000) {
    display_draw_text(fb, "!!Critical!!", 128 / 3 - 24, 42, wm, rot);
  } else if (display_info->co2_measurement > 1500) {
    display_draw_text(fb, "Bad", 128 / 3 + 13, 42, wm, rot);
  } else if (display_info->co2_measurement > 1000) {
    display_draw_text(fb, "Okay", 128 / 3 + 11, 42, wm, rot);
  } else if (display_info->co2_measurement > 400) {
    display_draw_text(fb, "Good", 128 / 3 + 11, 42, wm, rot);
  }

  display_send_buffer(fb);
}

enum STATUS wifi_status() {
  int status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);

  switch (status) {
  case CYW43_LINK_DOWN:
    printf("update_display_task: wifi_status: Wifi down.\n");
    break;
  case CYW43_LINK_JOIN:
    printf("update_display_task: wifi_status: Connected to wifi.\n");
    break;
  case CYW43_LINK_NOIP:
    printf("update_display_task: wifi_status: Connected to wifi, but no IP "
           "address.\n");
    break;
  case CYW43_LINK_UP:
    break;
  case CYW43_LINK_FAIL:
    printf("update_display_task: wifi_status: Connection failed.\n");
    break;
  case CYW43_LINK_NONET:
    printf("update_display_task: wifi_status: No matching SSID found.\n");
    break;
  case CYW43_LINK_BADAUTH:
    printf("update_display_task: wifi_status: Auth failure.\n");
    break;
  }

  if (status == CYW43_LINK_UP || status == CYW43_LINK_NOIP) {
    return STATUS_GOOD;
  }
  return STATUS_BAD;
}

void update_display_task(void *task_params) {
  printf("Hello from update_display_task!\n");
  update_display_params *params = (update_display_params *)task_params;

  // Other vars
  uint16_t co2 = 0;

  DisplayInfo display_info;
  display_info.wifi_status = STATUS_GOOD;
  display_info.sensor_status = STATUS_GOOD;
  display_info.co2_measurement = 0; // Temp value

  update_display(&display_info, params->fb, params->wm, params->rot);
  uint32_t receivedCommand;

  while (1) {
    if (xTaskNotifyWait(0x00, ULONG_MAX, &receivedCommand,
                        pdMS_TO_TICKS(500)) == pdPASS) {
      switch (receivedCommand) {
      case 2:
        printf("update_display_task: Received CMD_DISPLAY_ON\n");
        oled_set_brightness(0xFF);
        break;
      case 1:
        printf("update_display_task: Received CMD_DISPLAY_OFF\n");
        oled_set_brightness(0);
        break;
      default:
        printf("update_display_task: Received unknown command: %d\n",
               receivedCommand);
      }
    }

    if (xQueueReceive(display_queue, &co2, portMAX_DELAY) == pdPASS) {
      display_info.wifi_status = wifi_status();
      display_info.co2_measurement = co2;

      update_display(&display_info, params->fb, params->wm, params->rot);
    }
  }
}

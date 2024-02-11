#include <FreeRTOS.h>
#include <queue.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <task.h>

#include "i2c/i2c.h"
#include "network/wifi.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "ssd1306/display.h"
#include "ssd1306/ssd1306.h"
#include "tasks/read_data_task.h"
#include "tasks/update_display_task.h"

// Check these definitions where added from the makefile
#ifndef WIFI_SSID
#error "WIFI_SSID not defined"
#endif
#ifndef WIFI_PASSWORD
#error "WIFI_PASSWORD not defined"
#endif

volatile QueueHandle_t queue = NULL;
TaskHandle_t read_data_handle = NULL;
TaskHandle_t update_display_handle = NULL;

int main() {
  stdio_init_all();

  sleep_ms(2000);  // Wait for serial_port to be initialized

  const char* ssid = WIFI_SSID;
  const char* password = WIFI_PASSWORD;

  if (!ssid || !password) {
    printf("main: SSID or Wi-Fi password not provided!\n");
    return -1;
  }

  // Wifi stuff
  if (wifi_init()) {
    printf("main: Wifi controller init.\n");
  } else {
    printf("main: Failed to initialize Wifi Controller\n");
  }

  if (wifi_join(WIFI_SSID, WIFI_PASSWORD, 5)) {
    printf("main: Connected to WIFI\n");
  } else {
    printf("main: Failed to connect to wifi\n");
  }

  // Print MAC Address
  char macStr[20];
  wifi_get_mac_address_str(macStr);
  printf("main: MAC ADDRESS: %s\n", macStr);

  // Print IP Address
  char ipStr[20];
  wifi_get_ip_address_str(ipStr);
  printf("main: IP ADDRESS: %s\n", ipStr);

  // Init display
  init_i2c();
  i2c_scan();

  oled_init();

  FrameBuffer fb;
  if (fb_init(&fb) != 0) {
    printf("update_display_task: Init failed, fb init failed.\n");
    return -1;
  }
  fb_clear(&fb);

  enum WriteMode wm = ADD;
  enum Rotation rot = deg0;
  update_display_params ud_params;
  ud_params.fb = &fb;
  ud_params.wm = wm;
  ud_params.rot = rot;
  printf("main: Creating Tasks\n");
  BaseType_t read_data_status = xTaskCreate(read_data_task, "READ_DATA_TASK",
                                            2056, NULL, 2, &read_data_handle);

  BaseType_t update_display_status =
      xTaskCreate(update_display_task, "UPDATE_DISPLAY_TASK", 4112,
                  (void*)&ud_params, 1, &update_display_handle);

  printf("main: Creating queue\n");
  queue = xQueueCreate(5, sizeof(int));

  if (read_data_status == pdPASS && update_display_status == pdPASS) {
    printf("main: Starting scheduler!\n");
    vTaskStartScheduler();
  } else {
    printf("main: Unable to start scheduler! RD: %s UD: %s", read_data_status,
           update_display_status);
    return -1;
  }
  // should never be reached
  while (1)
    ;
}

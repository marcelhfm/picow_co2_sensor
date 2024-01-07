#include "read_data_task.h"

#include <FreeRTOS.h>
#include <queue.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <task.h>

#include "../scd40/scd40.h"
#include "pico/stdlib.h"

extern QueueHandle_t queue;

const TickType_t read_data_delay = 5000 / portTICK_PERIOD_MS;

void read_data_task() {
  printf("Hello from read_data_task!\n");

  int16_t retval = scd40_init();

  if (retval != 0) {
    printf("read_data_task: Error initializing task: %i\n", retval);
    return;
  }

  uint16_t co2;
  int32_t temp_mc;
  int16_t humidity_m_percent_rh;

  while (1) {
    bool data_ready_flag = false;

    retval = scd40_get_data_ready_flag(&data_ready_flag);

    if (retval != 0) {
      printf("read_data_task: Error executing get_data_ready_flag: %i\n",
             retval);
      continue;
    }
    if (!data_ready_flag) {
      continue;
    }

    retval = scd40_read_measurement(&co2, &temp_mc, &humidity_m_percent_rh);
    if (retval != 0) {
      printf("read_data_task: error executing read_measurement: %i\n", retval);
    } else if (co2 == 0) {
      printf("read_data_task: invalid sample detected, skipping.\n");
    } else {
      printf("C02: %u ppm\n", co2);
      printf("Temperature: %i\n", temp_mc);
      printf("humidity_m_percent_rh: %i\n", humidity_m_percent_rh);
    }

    xQueueSendToBack(queue, &co2, 0);
    vTaskDelay(read_data_delay);
  }
}

#include "cyw43_ll.h"
#include "lwip/err.h"
#include "lwip/ip4_addr.h"
#include "lwip/tcpbase.h"
#include "pico/cyw43_arch.h"

#include "../main.h"
#include "lwip/tcp.h"
#include "network_task.h"
#include "projdefs.h"
#include <FreeRTOS.h>
#include <queue.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <task.h>

extern QueueHandle_t network_queue;

static err_t tcp_sent_callback(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t tcp_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err);
static void tcp_error_callback(void *arg, err_t err);
static void try_tcp_send(struct tcp_pcb *tpcb, const char *data);

static struct tcp_pcb *tcp_pcb;

static void tcp_client_init() {
  ip_addr_t server_ip;
  tcp_pcb = tcp_new();
  if (!tcp_pcb) {
    printf("Error creating PCB.\n");
    return;
  }

  IP4_ADDR(&server_ip, 192, 168, 10, 21);
  printf("network_task: Attempting to bind and connect to server at %s:%d\n",
         ip4addr_ntoa(&server_ip), SERVER_PORT);

  if (tcp_bind(tcp_pcb, IP_ADDR_ANY, 0) != ERR_OK) {
    printf("network_task: Error binding TCP.\n");
    tcp_abort(tcp_pcb);
    return;
  }

  tcp_connect(tcp_pcb, &server_ip, SERVER_PORT, tcp_connected_callback);
}

static err_t tcp_connected_callback(void *arg, struct tcp_pcb *tpcb,
                                    err_t err) {
  printf("network_task: Attempting to connect...\n");
  if (err == ERR_OK) {
    tcp_sent(tpcb, tcp_sent_callback);
    tcp_err(tpcb, tcp_error_callback);

    return ERR_OK;
  } else {
    printf("network_task: Connection failed with error code %d\n", err);
    return err;
  }
}

static err_t tcp_sent_callback(void *arg, struct tcp_pcb *tpcb, u16_t len) {
  printf("network_task: Data sent Successfully.\n");
  return ERR_OK;
}

static void tcp_error_callback(void *arg, err_t err) {
  printf("network_task: TCP connection error %d encountered. Restoring "
         "connection\n",
         err);

  struct tcp_pcb *tpcb = (struct tcp_pcb *)arg;
  if (tpcb) {
    tcp_pcb = NULL;
    tcp_close(tpcb);
  }

  vTaskDelay(pdMS_TO_TICKS(5000));
  tcp_client_init();
}

static void try_tcp_send(struct tcp_pcb *tpcb, const char *data) {
  if (tpcb && tpcb->state == ESTABLISHED) {
    err_t err = tcp_write(tpcb, data, strlen(data), TCP_WRITE_FLAG_COPY);
    if (err == ERR_OK) {
      tcp_output(tpcb);
    } else {
      printf("network_task: Failed to send data: %d\n", err);
    }
  } else {
    printf("network_task: Connection lost, attempting reconnect...\n");
    tcp_client_init();

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

void network_task() {
  printf("Hello from network_task!\n");

  if (cyw43_arch_init()) {
    printf("network_task: Failed to initialize CYW43\n");
    vTaskDelete(NULL);
  }

  cyw43_arch_enable_sta_mode();

  printf("network_task: Connecting to WiFi %s and pwd %s\n", WIFI_SSID,
         WIFI_PASSWORD);

  while (cyw43_arch_wifi_connect_timeout_ms(
             WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0) {
    printf("network_task: Failed to connect to WiFi, retrying...\n");
    vTaskDelay(pdMS_TO_TICKS(5000));
  }

  printf("network_task: Successfully connected to wifi!\n");

  tcp_client_init();

  measurements_t data;
  char buffer[100];

  while (1) {
    if (xQueueReceive(network_queue, &data, portMAX_DELAY) == pdPASS) {
      if (snprintf(buffer, sizeof(buffer), "1,%d,%d,%d\n", data.co2, data.temp,
                   data.humidity) >= sizeof(buffer)) {
        printf("network_task: Buffer overflow detected!\n");
      }

      try_tcp_send(tcp_pcb, buffer);
    }
  }

  cyw43_arch_deinit();
  vTaskDelete(NULL);
}

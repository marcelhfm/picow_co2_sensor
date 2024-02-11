#ifndef WIFI_C
#define WIFI_C

#include <stdint.h>

bool wifi_init();

bool wifi_join(const char *sid, const char *password, uint8_t retries);

bool wifi_get_ip_address(uint8_t *ip);

bool wifi_get_ip_address_str(char *ips);

bool wifi_get_gw_address(uint8_t *ip);

bool wifi_get_gw_address_str(char *ips);

bool wifi_get_net_mask(uint8_t *ip);

bool wifi_get_net_mask_str(char *ips);

bool wifi_get_mac_address_str(char *macStr);

bool wifi_is_joined();

#endif  // !WIFI_C

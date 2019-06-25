#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <lwip/sys.h>
#include "macros.h"


static esp_err_t nvs_init() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ERET( nvs_flash_erase() );
    ERET( nvs_flash_init() );
  }
  ERET( ret );
  return ESP_OK;
}


static void on_wifi(void* arg, esp_event_base_t base, int32_t id, void* data) {
  if (id == WIFI_EVENT_STA_CONNECTED) {
    wifi_event_sta_connected_t *d = (wifi_event_sta_connected_t*) data;
    printf("- Connected to %s AP (channel %d) (event)\n", d->ssid, d->channel);
  } else if (id == WIFI_EVENT_STA_DISCONNECTED) {
    wifi_event_sta_disconnected_t *d = (wifi_event_sta_disconnected_t*) data;
    printf("- Disconnected from AP %s (event)\n", d->ssid);
  }
}


static void on_ip(void *arg, esp_event_base_t base, int32_t id, void *data) {
  if (id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *d = (ip_event_got_ip_t*) data;
    printf("- Got IP %s (event)\n", ip4addr_ntoa(&d->ip_info.ip));
  }
}


static esp_err_t wifi_sta() {
  printf("- Initialize TCP/IP adapter\n");
  tcpip_adapter_init();
  printf("- Create default event loop\n");
  ERET( esp_event_loop_create_default() );
  printf("- Initialize WiFi with default config\n");
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ERET( esp_wifi_init(&cfg) );
  printf("- Register WiFi event handler\n");
  ERET( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &on_wifi, NULL) );
  printf("- Register IP event handler\n");
  ERET( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_ip, NULL) );
  printf("- Set WiFi mode as station\n");
  ERET( esp_wifi_set_mode(WIFI_MODE_STA) );
  printf("- Set WiFi configuration\n");
  wifi_config_t wifi_config = {.sta = {
    .ssid = "belkin.a58",
    .password = "ceca966f",
    .scan_method = WIFI_ALL_CHANNEL_SCAN,
    .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
    .threshold.rssi = -127,
    .threshold.authmode = WIFI_AUTH_OPEN,
  }};
  ERET( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
  printf("- Start WiFi\n");
  ERET( esp_wifi_start() );
  printf("- Connect to set AP\n");
  ERET( esp_wifi_connect() );
  return ESP_OK;
}


void app_main() {
  printf("- Initialize NVS\n");
  ESP_ERROR_CHECK( nvs_init() );
  ESP_ERROR_CHECK( wifi_sta() );
}

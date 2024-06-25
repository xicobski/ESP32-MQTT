#include <string.h>
#include <stdio.h>
#include <esp_system.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "wifi.h"

static EventGroupHandle_t evgWifi;
static esp_netif_t* staNetifhandler;

static void wifiEventHandler(void* args, esp_event_base_t base, int32_t id, void* data);

esp_err_t nvsInit() {
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		nvs_flash_erase();
		ret = nvs_flash_init();
	}
	return ret;
}

void connectWifi(const char* ssid, const char* pwd) {
	evgWifi = xEventGroupCreate();

	esp_netif_init();
	esp_event_loop_create_default();
	staNetifhandler = esp_netif_create_default_wifi_sta();

	wifi_init_config_t wifiCfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&wifiCfg);

	esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifiEventHandler, NULL);
	esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifiEventHandler, NULL);

	wifi_sta_config_t staCfg = {
			.scan_method = WIFI_FAST_SCAN
	};
	strcpy((char*)staCfg.password, pwd);
	strcpy((char*)staCfg.ssid, ssid);

	wifi_config_t cfg = {
			.sta = staCfg,
	};
	esp_wifi_set_mode(WIFI_MODE_STA);
	esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg);

	esp_wifi_start();

	EventBits_t bits = xEventGroupWaitBits(evgWifi, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

	if (bits & WIFI_CONNECTED_BIT) {
		printf("Connected to: %s\n", ssid);
	} else if (bits & WIFI_FAIL_BIT) {
		printf("Failed to connect to: %s\n", ssid);
	} else {
		printf("Unexpected event!!\n");
	}
}

static void wifiEventHandler(void* args, esp_event_base_t base, int32_t id, void* data) {
	static uint8_t retry = 0;
	if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
		if (retry < MAX_RETRY) {
			retry++;
			esp_wifi_connect();
		} else {
			xEventGroupSetBits(evgWifi, WIFI_FAIL_BIT);
		}
	} else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
		retry = 0;
		xEventGroupSetBits(evgWifi, WIFI_CONNECTED_BIT);
	}
}

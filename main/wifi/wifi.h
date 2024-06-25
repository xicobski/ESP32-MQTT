#include "esp_err.h"

#ifndef __wifi_h__
#define __wifi_h__

#define SSID "NOME_DA_REDE"
#define PWD  "PASSWORD"

#define MAX_RETRY 			(uint8_t)(3)
#define WIFI_CONNECTED_BIT 	(uint8_t)(1<<0)
#define WIFI_FAIL_BIT 		(uint8_t)(1<<1)

esp_err_t nvsInit();
void connectWifi(const char* ssid, const char* pwd);

#endif

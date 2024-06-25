#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "mqtt.h"
#include "wifi.h"

#define delay(x)(vTaskDelay((x) / portTICK_PERIOD_MS))

#define NTOPICS 	(uint8_t)(2)

void mqtt_callback(const char* topic, const char* data);

void app_main(void) {
	const char* topics[NTOPICS] = {"dev1", "dev2"};

	// Init WIFI and NVS
	nvsInit();
	connectWifi(SSID, PWD);

	// Init MQTT
	mqttSetTopicsNum(NTOPICS);
	mqttSetTopics(topics);
	mqttStart("localhost", MQTT_PORT, mqtt_callback);
    while (true) {
    	mqttPublish("dev1", "Message for dev1");
    	delay(1000);
    	mqttPublish("dev2", "Message for dev2");
    	delay(1000);
    }
}

void mqtt_callback(const char* topic, const char* data) {
	printf("\"%s\" was received @ topic: %s\n", data, topic);
}

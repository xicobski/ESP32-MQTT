#ifndef __MQTT_H__
#define __MQTT_H__

#include <stdint.h>
#include "mqtt_client.h"
#include "esp_err.h"

#define MQTT_PORT			(uint32_t)(1883)

void mqttStart(const char* ip, const uint32_t port, esp_err_t (*callback)(const char*, const char*));
void mqttPublish(const char* topic, const char* payload);
void mqttSetTopicsNum(const uint8_t N);
void mqttSetTopics(const char** topicList);

#endif




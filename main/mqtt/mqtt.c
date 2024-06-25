#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mqtt_client.h"
#include "mqtt.h"

#define MQTT_MAX_RETRY		(uint8_t)(3)

static uint8_t NUM_OF_TOPICS = 0;
static char* MQTT_IP;

static esp_mqtt_client_handle_t client;
static char** topics;
static esp_err_t (*mqttCallback)(const char* topic, const char* data);

static void mqttEventHandler(void* arg, esp_event_base_t base, int32_t id, void* ev);
static char* formatString(const char* str, const uint8_t len);

void initMqttClient(const char* ip, const uint32_t port, esp_err_t (*callback)(const char*, const char*)) {
	mqttCallback = callback;

	MQTT_IP = (char*)malloc(strlen(ip));
	strcpy(MQTT_IP, ip);

	esp_mqtt_client_config_t cfg = {
		.broker.address.hostname = ip,
		.broker.address.port = port,
		.broker.address.transport = MQTT_TRANSPORT_OVER_TCP,
		.credentials.set_null_client_id = true,
		.session.protocol_ver = MQTT_PROTOCOL_V_3_1_1,
		.task.priority = 1,
		.task.stack_size = 4096,
		.buffer.size = 1024,
		.buffer.out_size = 1024
	};
	client = esp_mqtt_client_init(&cfg);

	esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqttEventHandler, NULL);
}

void mqttConnect() {
	esp_mqtt_client_start(client);
}

void mqttPublish(const char* topic, const char* payload) {
	esp_mqtt_client_publish(client, topic, payload, 0, 2, 0);
}

void mqttSetTopicsNum(const uint8_t N) {
	NUM_OF_TOPICS = N;
	topics = (char**)malloc(N*sizeof(char*));
	printf("[mqtt] Number of topics defined to %d\n", N);
}

void mqttSetTopics(const char** topicList) {
	if (NUM_OF_TOPICS == 0) {
		printf("[mqtt] Error: Define the number of topics before defining the topics (call mqttSetTopicsNum)\n");
		return;
	}
	for (uint8_t t = 0; t < NUM_OF_TOPICS; t++) {
		topics[t] = (char*)malloc(strlen(topicList[t]));
		strcpy(topics[t], topicList[t]);
	}
	printf("[mqtt] All topics defined\n");
}

static void mqttEventHandler(void* arg, esp_event_base_t base, int32_t id, void* ev) {
	static uint8_t retry = MQTT_MAX_RETRY;
	esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)ev;
	esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)arg;
	char* topic = (char*)malloc(event->topic_len);
	char* data = (char*)malloc(event->data_len);

	if ((esp_mqtt_event_id_t)id == MQTT_EVENT_ERROR) {
			if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
				printf("[mqtt] Error: TCP Transport!\n");
			else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED)
				printf("[mqtt] Error: Connection Refused!\n");
			else if (event->error_handle->error_type == MQTT_ERROR_TYPE_SUBSCRIBE_FAILED)
				printf("[mqtt] Error: Subscribe Failed1\n");
			else
				printf("[mqtt] Error: Undefined Error!\n");
	} else if((esp_mqtt_event_id_t)id == MQTT_EVENT_BEFORE_CONNECT) {
			printf("[mqtt] Starting Connection @ %s:%d\n", MQTT_IP, (uint16_t)MQTT_PORT);
	} else if ((esp_mqtt_event_id_t)id == MQTT_EVENT_CONNECTED) {
			retry = MQTT_MAX_RETRY;
			esp_mqtt_topic_t list[NUM_OF_TOPICS];
			for (uint8_t t = 0; t < NUM_OF_TOPICS; t++) {
				list[t].filter = malloc(strlen(topics[t]));
				list[t].filter = topics[t];
				list[t].qos = 2;
			}
			esp_mqtt_client_subscribe_multiple(client, list, NUM_OF_TOPICS);
	} else if((esp_mqtt_event_id_t)id == MQTT_EVENT_DISCONNECTED) {
			if (--retry) {
				printf("[mqtt] Retrying connection (%d/%d)\n", retry+1, MQTT_MAX_RETRY);
				esp_mqtt_client_start(client);
			} else {
				printf("[mqtt] Reconnection Failed!\n");
			}
	} else if ((esp_mqtt_event_id_t)id == MQTT_EVENT_PUBLISHED) {
			topic = formatString(event->topic, event->topic_len);
			data = formatString(event->data, event->data_len);
			printf("[mqtt] Published message: %s @ topic: %s\n", data, topic);
	} else if ((esp_mqtt_event_id_t)id == MQTT_EVENT_SUBSCRIBED) {
			printf("Subsribed to all topics:");
			for (uint8_t t = 0; t < NUM_OF_TOPICS; t++) {
				printf("	- %s\n", topics[t]);
			}
	} else if ((esp_mqtt_event_id_t)id == MQTT_EVENT_UNSUBSCRIBED) {
			topic = formatString(event->topic, event->topic_len);
			printf("Unsubscribed from %s\n", topic);
	} else if ((esp_mqtt_event_id_t)id == MQTT_EVENT_DATA) {
			topic = formatString(event->topic, event->topic_len);
			data = formatString(event->data, event->data_len);

			mqttCallback(topic, data);
	} else {
			printf("[mqtt] Undefined event: %d\n", (uint8_t)id);
	}
}

static char* formatString(const char* str, const uint8_t len) {
	char* ret = (char*)malloc(len * sizeof(char));
	sprintf(ret, "%.*s", len, str);
	return ret;
}

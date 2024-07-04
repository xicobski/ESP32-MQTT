# ESP32-MQTT
---

This implementation was made following the [Espressif Programming Guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/protocols/mqtt.html) with the goal of abtracting the use of the esp mqtt library.

---
## Dependencies
Dependencies to add to your CMakeLists' REQUIRES file:
1. driver
2. freertos
3. esp_wifi
4. nvs_flash
5. mqtt

Dependencies to add to your CMakeLists' SRCS:
1. wifi/wifi.c
2. mqtt/mqtt.c

---
## How to use this module

#### WiFi Initialization
- To connect the ESP32 to the wifi, first initialize NVS and then call the connectWifi function:

```c
void app_main() {
    nvsInit();
    connectWifi(SSID, PASSWORD);
    // ...
}
```

- This requires the *wifi.h* file to be included:

```c
#include "wifi.h"
```

#### MQTT Initialization
- To initialize this module we define the amount of topics the ESP32 needs to subscribe, then we set the topics it will subscribe after connecting, and finally we start the connection, passing the **ip** and **port** of the MQTT broker and a callback function that handles the messages received:  

```c
void app_main() {
    // ...

    const char* topics[2] = {"topic1", "topic2"};
    mqttSetTopicsNum(2);   // set the amount of topics to subscribe
    mqttSetTopics(topics); // give the topics to subscribe after connection
    mqttStart(IP, PORT, mqtt_callback); // give ip and port of the mqtt broker and a callback function to handle received messages
    
    // ...
}
```

- This requires the *mqtt.h* file to be included:

```c
#include "mqtt.h"
```

#### MQTT Callback
- This code implements a callback function when a message is received. This function receives the message's topic and payload:

```c
void mqtt_callback(const char* topic, const char* data) {
    // Message handling
}
```

#### MQTT Publish
- To publish a message just call the mqttPublish function and give it a topic and the data to publish:

```c
mqttPublish("topic1", "Message for topic 1");
```

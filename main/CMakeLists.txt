# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS main.c wifi/wifi.c mqtt/mqtt.c
    INCLUDE_DIRS .
    PRIV_INCLUDE_DIRS
    REQUIRES driver freertos esp_wifi nvs_flash mqtt
    PRIV_REQUIRES
)

idf_component_register(
    SRCS "nvs_manager.c" "wifi.c" "httpServer.c" "gustavianWifi.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_driver_gpio nvs_flash log freertos esp_wifi esp_event esp_netif esp_http_client esp_http_server cjson)
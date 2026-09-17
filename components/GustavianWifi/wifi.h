#ifndef WIFI_H
#define WIFI_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

void task_provisionamentoWifiHTTP(void *parameters);

int conectar_ultima_rede();

void conectar_rede(char* ssid, char* senha);

void configurar_wifi();

#endif
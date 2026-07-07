#ifndef GUSTAVIANWIFI_H
#define GUSTAVIANWIFI_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_manager.h"
#include "wifi.h"
#include "httpServer.h"
#include "sdkconfig.h"
#include "driver/gpio.h"

static void IRAM_ATTR isr_botao(void *arg);
void task_intr_wifi(void *parameters);
void gustavianWifiStart();


#endif
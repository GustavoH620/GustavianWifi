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
#include "gustavianWifi.h"


void app_main(void)
{
    gustavianWifiStart();
    
}

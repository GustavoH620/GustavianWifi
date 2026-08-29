#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_manager.h"
#include "wifi.h"
#include "eventos.h"
#include "httpServer.h"
#include "sdkconfig.h"
#include "driver/gpio.h"


extern EventGroupHandle_t eventos_status;
//FILA E INTERRUPÇÃO

QueueHandle_t fila_botao_isr;

static void IRAM_ATTR isr_botao(void *arg){
    int btn = (int)arg;
    bool nivel = gpio_get_level(CONFIG_GPIO_INTR);
    xQueueSendFromISR(fila_botao_isr, &nivel, NULL);

}

// tasks e variáveis

bool conexao = false;

void task_intr_wifi(void *parameters){
    int estado_btn;
    TickType_t tempoAnterior = 0;
    for (;;){
        xQueueReceive(fila_botao_isr, &estado_btn, portMAX_DELAY);
        TickType_t tempoAtual = xTaskGetTickCount();
        if ((tempoAtual - tempoAnterior) > pdMS_TO_TICKS(1000) && !estado_btn){
            ESP_LOGI("Task INTR", "Interrupção recebida, inicianddo provisionamento...");
            tempoAnterior = tempoAtual;
            xEventGroupSetBits(eventos_status, PROVISIONING_STATUS);
            xTaskCreate(task_provisionamentoWifiHTTP, "task Provisionamento", 2048, NULL, 2, NULL);
        }
    }
}

bool checar_conexao(){
    EventBits_t bits = xEventGroupGetBits(eventos_status);
    if (bits & CONEXAO_STATUS){
        return true;
    } else {
        return false;
    }
}

void gustavianWifiStart(){

    gpio_config_t config_botao = {
        .pin_bit_mask = (1ULL << CONFIG_GPIO_INTR),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&config_botao);

    fila_botao_isr = xQueueCreate(5, sizeof(int));
    gpio_install_isr_service(0);
    gpio_isr_handler_add(CONFIG_GPIO_INTR, isr_botao, (void*) CONFIG_GPIO_INTR);

    criar_eventGroup();
    ESP_LOGI("MAIN", "Event Group criado");
    iniciar_nvs();
    ESP_LOGI("MAIN", "NVS iniciado");
    configurar_wifi();

    vTaskDelay(pdMS_TO_TICKS(1000));
    int ultima_rede = checar_ultima_rede();
    if (ultima_rede){
        ESP_LOGI("MAIN", "Última rede não encontrada, iniciando provisionamento...");
        xEventGroupSetBits(eventos_status, PROVISIONING_STATUS);
        xTaskCreate(task_provisionamentoWifiHTTP, "task Provisionamento", 2048, NULL, 2, NULL);

    } else {
        conectar_ultima_rede();
    }
    

    

    if (checar_conexao()) {
        ESP_LOGI("MAIN", "Rede conectada!, continuando rotina...");
    } else {
        ESP_LOGI("MAIN", "Rede não conectada...");
    }

    xTaskCreate(
        task_intr_wifi,
        "Task INTR",
        2048,
        NULL,
        2,
        NULL
    );

}




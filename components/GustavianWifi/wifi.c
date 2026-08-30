#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_manager.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_netif.h"
#include "httpServer.h"
#include "eventos.h"

extern httpd_handle_t servidor;
extern credenciais_status_wifi ultima_rede;

const char* TAG = "WIFI";
int8_t contadorTentativas = 0;
int tempo_retry = 2000;

void configurar_wifi(){
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));
    ESP_ERROR_CHECK(esp_wifi_start());

}

void task_callback_disconexao(void *params){
    EventBits_t bits = xEventGroupGetBits(eventos_status);
    if ((bits & PROVISIONING_STATUS) == 0){
        ESP_LOGW(TAG, "Conexão falhou!");
        contadorTentativas++;
        if (contadorTentativas < CONFIG_NT_TENTATIVAS || !CONFIG_BOOL_RECX){
            if (contadorTentativas > 1) ESP_LOGW(TAG, "Conexão falhou novamente...\n Tentativas: %d", contadorTentativas);
            ESP_ERROR_CHECK(esp_wifi_connect());
            vTaskDelay(pdMS_TO_TICKS(tempo_retry));
            if (tempo_retry < 30000) tempo_retry = tempo_retry * 2;
            if (tempo_retry > 30000) tempo_retry = 30000;

        } else {
            ESP_LOGW(TAG, "Tentativa de reconexão falhou, iniciando provisionamento...");
            EventBits_t bits = xEventGroupGetBits(eventos_status);
            if (bits & PROVISIONING_STATUS) {
                ESP_LOGW("PROVISIONAMENTO", "Erro: provisionamento já iniciado");
            } else {
                xEventGroupSetBits(eventos_status, PROVISIONING_STATUS);
                xTaskCreate(task_provisionamentoWifiHTTP, "task Provisionamento", 2048, NULL, 2, NULL);
            }
            
        }
    } else {
        ESP_LOGI(TAG, "Provisionamento intencional");
    }
    vTaskDelete(NULL);
}
void task_provisionamentoWifiHTTP(void *parameters)
{
    EventBits_t bits = xEventGroupGetBits(eventos_status);
    if (bits & PROVISIONING_STATUS){
        ESP_LOGI("WIFI", "Aqui é o método de provisionamento!");
        esp_wifi_disconnect();

        //Definir os dados da rede aberta que o ESp32 vai criar
        wifi_config_t ap_config = {
            .ap = {
                .ssid="Gustavian ESP",
                .ssid_len = strlen("Gustavian ESP"),
                .channel = 1,
                .password = "123#45gG", //Deixa vazio para rede aberta
                .max_connection = 4,
                .authmode = WIFI_AUTH_WPA3_PSK
            },
        };

        //Configura o chip para atuar como Access Point (Roteador) e Station (Cliente) ao mesmo tempo
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

        //Aplica as configurações da rede
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));

        //Liga o rádio Wi-fi

        if ((bits & WIFI_STATUS) == 0) ESP_ERROR_CHECK(esp_wifi_start());
        xEventGroupWaitBits(eventos_status, WIFI_STATUS, pdFALSE, pdTRUE, pdMS_TO_TICKS(10000));
        ESP_LOGI("WIFI", "Rádio wifi ligado");
        //Agora que a rede está no ar (IP 192.168.4.1), podemos iniciar o servidor
        servidor = inicializar_servidor_web();
        
    } else {
        ESP_LOGI("WIFI", "Provisionamento já iniciado");
    }
    vTaskDelete(NULL);

}

int conectar_ultima_rede(){
    EventBits_t bits = xEventGroupGetBits(eventos_status);
    int check_ultima_rede = checar_ultima_rede();
    if (check_ultima_rede == 0){
        xEventGroupWaitBits(eventos_status, WIFI_STATUS, pdFALSE, pdTRUE, pdMS_TO_TICKS(15000));
        wifi_config_t wifi_config = {0};
        strlcpy((char*) wifi_config.sta.ssid, ultima_rede.ssid, sizeof(wifi_config.sta.ssid));
        strlcpy((char*) wifi_config.sta.password, ultima_rede.senha, sizeof(wifi_config.sta.password));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_connect());
        ESP_LOGI("WIFI", "Tentando se conectar a última rede...");
        return 0;
        


    } else {

        ESP_LOGI("WIFI", "Última rede não encontrada");
        return 1;
        
        
    }
}

void conectar_rede(char* ssid, char* senha){
    wifi_config_t wifi_config = {0};
    strlcpy((char*) wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strlcpy((char*) wifi_config.sta.password, senha, sizeof(wifi_config.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
    ESP_LOGI("WIFI", "Tentando se conectar a rede...");

}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    EventBits_t bits = xEventGroupGetBits(eventos_status);
    //Evento 1: O wi-fi acabou de ser ligado (Iniciado)
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START){
        ESP_LOGI(TAG, "Wifi iniciado! Mudando event group...");
        xEventGroupSetBits(eventos_status, WIFI_STATUS);

    }
    //Evento 2: A conexão falhou (senha errada, roteador longe, etc)
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED){
        xEventGroupClearBits(eventos_status, WIFI_STATUS | CONEXAO_STATUS);
        xTaskCreate(task_callback_disconexao, "Handler de disconexão", 2048, NULL, 2, NULL);

    }
    //Evento 3: conexão bem sucedida, endereço IP recebido
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Conexão realizada com sucesso! IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(eventos_status, CONEXAO_STATUS);
        contadorTentativas = 0;
        httpd_stop(servidor);
        esp_wifi_set_mode(WIFI_MODE_STA);
        xEventGroupClearBits(eventos_status, PROVISIONING_STATUS);

    }


}


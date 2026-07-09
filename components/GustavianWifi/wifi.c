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

typedef struct {
    char ssid[64];
    char senha[32];
    bool ultima_acessada;
} credenciais_status_wifi;

extern httpd_handle_t servidor;
extern credenciais_status_wifi ultima_rede;
extern bool conexao;
volatile bool wifi_iniciado = false;
volatile bool provisionamento = false;
const char* TAG = "WIFI";
int8_t contadorTentativas = 0;
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    //Evento 1: O wi-fi acabou de ser ligado (Iniciado)
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START){
        ESP_LOGI(TAG, "Wifi iniciado!");
        wifi_iniciado = true;
        

        //esp_wifi_connect(); //tenta conectar
    }
    //Evento 2: A conexão falhou (senha errada, roteador longe, etc)
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED){
        conexao = false;
        if (!provisionamento){
            ESP_LOGW(TAG, "Conexão falhou!");
            contadorTentativas++;
            if (contadorTentativas < 16 || !CONFIG_BOOL_RECX){
                if (contadorTentativas > 1) ESP_LOGW(TAG, "Conexão falhou novamente...\n Tentativas: %d", contadorTentativas);
                ESP_ERROR_CHECK(esp_wifi_connect());

            } else {
                ESP_LOGW(TAG, "Tentativa de reconexão falhou, inicianddo provisionamento...");
                provisionamentoWifiHTTP();
            }
        } else {
            ESP_LOGI(TAG, "Provisionamento intencional");
        }
    }
    //Evento 3: conexão bem sucedida, endereço IP recebido
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Conexão realizada com sucesso! IP: " IPSTR, IP2STR(&event->ip_info.ip));
        conexao = true;
        contadorTentativas = 0;
        httpd_stop(servidor);
        esp_wifi_set_mode(WIFI_MODE_STA);

    }


}

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
void provisionamentoWifiHTTP()
{
    if (!provisionamento){
        provisionamento = true;
        ESP_LOGI("WIFI", "Aqui é o método de provisionamento!");
        esp_wifi_disconnect();

        //Definir os dados da rede aberta que o ESp32 vai criar
        wifi_config_t ap_config = {
            .ap = {
                .ssid="Gustavian ESP",
                .ssid_len = strlen("Gustavian ESP"),
                .channel = 1,
                .password = "", //Deixa vazio para rede aberta
                .max_connection = 4,
                .authmode = WIFI_AUTH_OPEN
            },
        };

        //Configura o chip para atuar como Access Point (Roteador) e Station (Cliente) ao mesmo tempo
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

        //Aplica as configurações da rede
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));

        //Liga o rádio Wi-fi
        if (!conexao) ESP_ERROR_CHECK(esp_wifi_start());
        vTaskDelay(pdMS_TO_TICKS(500));
        ESP_LOGI("WIFI", "Rádio wifi ligado");
        //Agora que a rede está no ar (IP 192.168.4.1), podemos iniciar o servior
        servidor = inicializar_servidor_web();
        
    } else {
        ESP_LOGI("WIFI", "Provisionamento já iniciado");
    }

}

int conectar_ultima_rede(){
    int check_ultima_rede = checar_ultima_rede();
    if (check_ultima_rede == 0){
        while (wifi_iniciado == false){
            vTaskDelay(pdMS_TO_TICKS(50));
        }
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
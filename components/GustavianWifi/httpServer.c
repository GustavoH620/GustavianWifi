#include <stdio.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "nvs_manager.h"
#include "wifi.h"
#include "eventos.h"



static httpd_handle_t servidor;
//bool info_incorreta = false;


const char* pagina_html =
    "<!DOCYPE html><html><body>"
    "<h2>Configurar Wi-Fi</h2>"
    "<form action=\"/salvar\" method=\"POST\">"
    "SSID: <input type=\"text\" name=\"ssid\"maxlenght=64\"><br><br>"
    "Senha: <input type=\"password\" name=\"senha\"maxlenght=32\"><br><br>"
    "<input type=\"submit\" value=\"Conectar\">"
    "</form></body></html>";
const char* pagina_html_erro =
        "<!DOCYPE html><html><body>"
        "<h2>Configurar Wi-Fi</h2>"
        "<h3 style='color: red;'>Rede ou Senha Incorretas!</h3>"
        "<form action=\"/salvar\" method=\"POST\">"
        "SSID: <input type=\"text\" name=\"ssid\"maxlenght=64\"><br><br>"
        "Senha: <input type=\"password\" name=\"senha\"maxlenght=32\"><br><br>"
        "<input type=\"submit\" value=\"Conectar\">"
        "</form></body></html>";




//GET
static esp_err_t rota_raiz_get(httpd_req_t *req) {
    httpd_resp_send(req, pagina_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

//POST
static esp_err_t rota_salvar_post(httpd_req_t *req){
    char buffer[250];

    
    int ret = httpd_req_recv(req, buffer, sizeof(buffer) - 1);
    if (ret <= 0){
        return ESP_FAIL;
    }
    buffer[ret] = '\0'; 

    ESP_LOGI("HTTP", "Texto recebido do formulário: %s", buffer);

    char ssid[64] = {0};
    char senha[32] = {0};
    int leituras = 0;
    
    leituras = sscanf(buffer, "ssid=%64[^&]&senha=%32[^\r\n]", ssid, senha);

    if (leituras == 2) {
        //ESP_LOGI("PARSER", "Nome extraido: %s \n", ssid);
        //ESP_LOGI("PARSER", "Senha extraída: %s \n", senha);

    } else {
        ESP_LOGW("PARSER", "Erro ao extrair dados");
        httpd_resp_send(req, pagina_html_erro, HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
        
    }
    
    
    ESP_LOGI("WIFI", "Tentando se conectar...");
    xEventGroupClearBits(eventos_status, CONEXAO_STATUS);
    conectar_rede(ssid, senha);
    xEventGroupWaitBits(eventos_status, CONEXAO_STATUS, pdFALSE, pdTRUE, pdMS_TO_TICKS(15000));
    EventBits_t bits = xEventGroupGetBits(eventos_status);
    if (bits & CONEXAO_STATUS){
        httpd_resp_send(req, "<h2>Conectado a rede!</h2>",HTTPD_RESP_USE_STRLEN);
        salvar_rede(ssid, senha);
    } else {
        
        
        httpd_resp_send(req, pagina_html_erro, HTTPD_RESP_USE_STRLEN);
        
    }

    return ESP_OK;

    
}

//4. Função para ligar o serviddor e registrar as portas
httpd_handle_t inicializar_servidor_web(void){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    servidor = NULL;

    if (httpd_start(&servidor, &config) == ESP_OK) {
        //Registra a rota da página inicial
        httpd_uri_t uri_raiz = {
            .uri = "/", .method = HTTP_GET, .handler = rota_raiz_get, .user_ctx = NULL

        };
        httpd_register_uri_handler(servidor, &uri_raiz);

        //Registra a rota de recepção dos dados
        httpd_uri_t uri_salvar = {
            .uri="/salvar", .method = HTTP_POST, .handler = rota_salvar_post, .user_ctx = NULL

        };
        httpd_register_uri_handler(servidor, &uri_salvar);

        ESP_LOGI("HTTP", "Servidor Web iniciado com sucesso");
    } else {
        EventBits_t bits = xEventGroupGetBits(eventos_status);
        if (bits & PROVISIONING_STATUS) {
            ESP_LOGW("PROVISIONAMENTO", "Erro: provisionamento já iniciado");
        } else {
            xEventGroupSetBits(eventos_status, PROVISIONING_STATUS);
            xTaskCreate(task_provisionamentoWifiHTTP, "task Provisionamento", 2048, NULL, 2, NULL);
        }
    }
    
    return servidor;
}

void parar_servidor_web(){
    httpd_stop(servidor);
}


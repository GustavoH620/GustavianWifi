#include <stdio.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "nvs_manager.h"
#include "wifi.h"

volatile httpd_handle_t servidor;
extern bool provisionamento;

typedef struct {
    char ssid[64];
    char senha[32];
    bool ultima_acessada;
} credenciais_status_wifi;

extern bool conexao;
//1. A página HML (String constante)
const char* pagina_html =
    "<!DOCYPE html><html><body>"
    "<h2>Configurar Wi-Fi</h2>"
    "<form action=\"/salvar\" method=\"POST\">"
    "SSID: <input type=\"text\" name=\"ssid\"><br><br>"
    "Senha: <input type=\"password\" name=\"senha\"><br><br>"
    "<input type=\"submit\" value=\"Conectar\">"
    "</form></body></html>";

//2. Rota GET: Envia a página para o navegador do utilizador
static esp_err_t rota_raiz_get(httpd_req_t *req) {
    httpd_resp_send(req, pagina_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

//3. Rota POST: Recebe os dados quando o utilizador clica em "conectar"
static esp_err_t rota_salvar_post(httpd_req_t *req){
    char buffer[100];

    //Lê o corpo da requisição HTTP
    int ret = httpd_req_recv(req, buffer, sizeof(buffer) - 1);
    if (ret <= 0){
        return ESP_FAIL;
    }
    buffer[ret] = '\0'; //Finaliza a string em C

    ESP_LOGI("HTTP", "Texto cru recebido co formulário: %s", buffer);
    //Nota: O navegador enviará algo como "ssid=MeuWifi&senha=MinhaSenha"
    //Teremos de separar (fazer o parse) desta string depois.

    //Envia uma resposta visual para o utilizador não ficar com o ecrã a carregar
    httpd_resp_send(req, "recebido! o ESP32 vai tentar ligar-se...", HTTPD_RESP_USE_STRLEN);

    vTaskDelay(pdMS_TO_TICKS(1000));

    char ssid[64] = {0};
    char senha[50] = {0};
    int leituras = 0;
    
    leituras = sscanf(buffer, "ssid=%50[^&]&senha=%50[^\r\n]", ssid, senha);

    if (leituras == 2) {
        ESP_LOGI("PARSER", "Nome extraido: %s \n", ssid);
        ESP_LOGI("PARSER", "Senha extraída: %s \n", senha);

    } else {
        ESP_LOGW("PARSER", "Erro ao extrair dados");
        return ESP_FAIL;
    }
    
    
    ESP_LOGI("WIFI", "Tentando se conectar...");
    provisionamento = false;
    conectar_rede(ssid, senha);
    vTaskDelay(pdMS_TO_TICKS(10000));
    if (conexao){
        httpd_resp_send(req, "Conectado a rede!",HTTPD_RESP_USE_STRLEN);
        salvar_rede(ssid, senha);
       
        
    } else {
        httpd_resp_send(req, "Informações incorretas", HTTPD_RESP_USE_STRLEN);
    }

    return ESP_OK;

    
}

//4. Função para ligar o serviddor e registrar as portas
httpd_handle_t inicializar_servidor_web(void){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t servidor = NULL;

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
        provisionamentoWifiHTTP();
    }
    
    return servidor;
}


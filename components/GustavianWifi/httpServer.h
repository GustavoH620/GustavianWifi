#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <stdio.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "wifi.h"

static esp_err_t rota_raiz_get(httpd_req_t *req);
static esp_err_t rota_salvar_post(httpd_req_t *req);
httpd_handle_t inicializar_servidor_web(void);


#endif
#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <stdio.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "wifi.h"

httpd_handle_t inicializar_servidor_web(void);


#endif
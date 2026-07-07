#ifndef NVS_MANAGER_H
#define NVS_MANAGER_H

#include <stdio.h>
#include "string.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

void iniciar_nvs();
int salvar_rede(char* ssid, char* senha);
int checar_ultima_rede();


#endif
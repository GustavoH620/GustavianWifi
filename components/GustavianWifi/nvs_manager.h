#ifndef NVS_MANAGER_H
#define NVS_MANAGER_H

#pragma once

#include <stdio.h>
#include "string.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

typedef struct {
    char ssid[64];
    char senha[32];
    bool ultima_acessada;
} credenciais_status_wifi;

extern credenciais_status_wifi ultima_rede;

void iniciar_nvs();
int salvar_rede(char* ssid, char* senha);
int checar_ultima_rede();


#endif
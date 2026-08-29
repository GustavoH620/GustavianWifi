#ifndef EVENTOS_H
#define EVENTOS_H

#pragma once 

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define WIFI_STATUS BIT0
#define CONEXAO_STATUS BIT1
#define PROVISIONING_STATUS BIT2

extern EventGroupHandle_t eventos_status;

void criar_eventGroup();

#endif
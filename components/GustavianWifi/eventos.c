#include "eventos.h"

EventGroupHandle_t eventos_status;

void criar_eventGroup(){
    eventos_status = xEventGroupCreate();
    xEventGroupClearBits(eventos_status, WIFI_STATUS | CONEXAO_STATUS | PROVISIONING_STATUS);
}

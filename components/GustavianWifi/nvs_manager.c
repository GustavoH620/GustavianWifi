#include "nvs_manager.h"




const char* particao_wifi = "part_wifi";
char buffer[100];
size_t tamanho_buffer = sizeof(buffer);
nvs_handle_t handle_nvs;
credenciais_status_wifi ultima_rede;


void iniciar_nvs(){
    ESP_ERROR_CHECK(nvs_flash_init());
    
    
}

int checar_ultima_rede(){
    ESP_ERROR_CHECK(nvs_open(particao_wifi, NVS_READWRITE, &handle_nvs));
    nvs_iterator_t iterador = NULL;
    esp_err_t erro = nvs_entry_find("nvs", particao_wifi, NVS_TYPE_BLOB, &iterador);

    while (erro == ESP_OK){
        nvs_entry_info_t info;
        nvs_entry_info(iterador, &info);
        credenciais_status_wifi credenciais;
        size_t tamanho_credenciais = sizeof(credenciais);
        esp_err_t erro_blob = nvs_get_blob(handle_nvs, "rede_salva", &credenciais, &tamanho_credenciais);
        if (erro_blob == ESP_OK){
            
            ESP_LOGI("NVS", "Namespace: %s\nChave: %s\nSSID: %s, Senha: %s, Ultima acessada?: %d", 
                    info.namespace_name, info.key, credenciais.ssid, credenciais.senha, credenciais.ultima_acessada);
            if (credenciais.ultima_acessada){
                ESP_LOGI("NVS", "Enviando credenciais encontradas para struct");
                ultima_rede = credenciais;
                nvs_close(handle_nvs);
                return 0;
            }
            
        } else if (erro_blob == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW("NVS", "Chave não encontrada");
            nvs_close(handle_nvs);
            return 1;

        } else {
            ESP_LOGW("NVS", "Erro ao tentar acessar dados: %s", esp_err_to_name(erro_blob));
            nvs_close(handle_nvs);
            return 1;
        }
        
        erro = nvs_entry_next(&iterador);


    }
    nvs_release_iterator(iterador);
    ESP_LOGI("NVS", "Última rede não encontrada");
    nvs_close(handle_nvs);
    return 1;



}
int salvar_rede(char* ssid, char* senha){
    ESP_ERROR_CHECK(nvs_open(particao_wifi, NVS_READWRITE, &handle_nvs));
    credenciais_status_wifi credenciais_novas;
    strcpy(credenciais_novas.ssid, ssid);
    strcpy(credenciais_novas.senha, senha);
    credenciais_novas.ultima_acessada = 1;
    size_t tamanho_credenciais = sizeof(credenciais_novas);
    ESP_ERROR_CHECK(nvs_set_blob(handle_nvs, "rede_salva", &credenciais_novas, tamanho_credenciais));
    nvs_commit(handle_nvs);
    ESP_LOGI("NVS", "Credenciais de última rede atualizadas");
    nvs_close(handle_nvs);
    return 0;
}


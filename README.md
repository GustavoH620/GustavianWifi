## Gustavian Wi-fi

Esse é meu gerenciador de conexão Wi-Fi que realiza a conexão e reconexão do ESP32 com redes Wi-Fi, algumas das funções são:

#### Fácil inicialização:
Apenas adicione gustavianWifiStart() em sua app_main e o serviço será iniciado.

#### Provisionamento por Access Point
Ou seja, cria uma rede e lança um servidor HTTP com uma página HTML para digitar o SSID e a senha da rede que deseja conectar.

#### Reconexão automática: 
Se a conexão for perdida, você tem a opção entre tentar reconectar infinitamente ou desistir e iniciar um provisionamento para outra rede.

#### Provisionamento a qualquer hora: 
É possível apertar um botão para acionar o provisionamento e mudar a rede a qualquer hora.

#### Persistência de dados: 
O gerenciador salva o SSID e senha da última rede que você conectou, e tenta se reconectar automaticamente após ser reiniciado, muito útil em caso de quedas de energia.

##### Informações técnicas
Para ESP-IDF 6.0+.

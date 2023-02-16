# FSE-Final-Project

## O que é este trabalho?

Este trabalho consiste na programação do dispositivo ESP32 para se comunicar via MQTT com o [ThingsBoard](https://thingsboard.io/). Este projeto conta com um sistema de leitura de temperatura e umidade através da comunicação com o sensor DHT11, e também com um sistema de leitura de presença magnética com o sensor KY-024.

## Como executar a aplicação

### Setup físico

Será necessário os equipamentos:

1. Uma ESP32 modelo Dev Kit v1
2. Sensor DHT11 digital
3. Sensor Hall de campo magnético KY-024.

Sobre a conectividade, será necessário:

1. Conectar a saída de dados do sensor DHT11 na GPIO5
2. Conectar a saída de dados do sensor KY-024 na GPIO4
3. Alimentar ambos sensores com 3.3v

### Setup de Software

Esta aplicação foi desenvolvida e testada utilizando o [Platform.io](https://platformio.org/), portanto, recomenda-se que se a plataforma seja instalada no [VSCode](https://code.visualstudio.com/).
O passo seguinte é clonar este reposítório para sua máquina local

```bash
git clone https://github.com/VictorJorgeFGA/FSE-Final-Project.git
```

E em seguida abrir o VSCode na raíz do projeto

```bash
code FSE-Final-Project
```

Dessa maneira, o Platform.io instalará automaticamente todas as dependencias atreladas ao projeto e fornecerá um menu de opções, dentre elas as opções de build, upload, monitor, etc.
Recomenda-se que seja feita a devida configuração das credênciais de WiFi e do servidor MQTT no menu config através das opções de configuração `Wifi Config` e `MQTT Config`, respectivamente.
Feitas as configurações, basta executar o `build` da aplicação e depois o `upload`. Para ver os logs de execução, recomenda-se utilizar a opção `monitor`.

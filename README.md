# ESP32 Pressure Sensor

Sistema de aquisição de dados IoT multicanal para monitoramento remoto de pressão e temperatura, desenvolvido com ESP32, conversores ADC externos ADS1115 e sensores industriais de pressão em malha de corrente 4–20 mA.

Este projeto foi desenvolvido como parte de um Trabalho de Conclusão de Curso em Engenharia Eletrônica, com aplicação no monitoramento de ensaios laboratoriais envolvendo pressão e temperatura em experimentos de produção de biometano.

## Visão Geral

O sistema realiza a leitura de até seis canais de pressão e um canal de temperatura, processando os sinais em um microcontrolador ESP32. As leituras são disponibilizadas por uma interface web local e podem ser acompanhadas por uma aplicação supervisória para Windows, responsável pela visualização e armazenamento dos dados em arquivo CSV.

A cadeia de aquisição dos canais de pressão é baseada em sensores industriais com saída de corrente de 4–20 mA. A corrente é convertida em tensão por meio de resistores shunt e posteriormente digitalizada por conversores ADS1115. No firmware, os dados são filtrados e convertidos para valores de pressão em unidade de engenharia.

## Funcionalidades

* Leitura de seis canais analógicos de pressão;
* Leitura de temperatura por sensor digital;
* Conversão de sinais 4–20 mA para pressão em bar;
* Filtragem digital das leituras;
* Comunicação Wi-Fi com interface web;
* Aplicação supervisória para Windows;
* Salvamento local dos dados em CSV;
* Projeto de hardware/PCB versionado junto ao firmware;
* Organização modular do firmware embarcado.

## Tecnologias Utilizadas

### Firmware

* ESP32;
* PlatformIO;
* Arduino Framework;
* Linguagem C/C++;
* Comunicação I2C;
* Conversores ADS1115;
* Sensor de temperatura DS18B20.

### Aplicação Supervisória

* Python;
* pywebview;
* requests;
* csv;
* threading.

### Hardware

* KiCad;
* Sensores industriais de pressão 4–20 mA;
* Resistores shunt;
* Conversores ADS1115;
* Módulo RTC;
* Módulo cartão SD;
* ESP32 DevKit.

## Estrutura do Repositório

```text
esp32-pressure-sensor/
├── App_PC/                  # Aplicação supervisória para Windows
├── Hardware/                # Projeto da PCB e arquivos de hardware
├── include/                 # Arquivos de cabeçalho do firmware
├── lib/                     # Bibliotecas internas do projeto
│   └── SensorLib/           # Biblioteca de aquisição e conversão dos sensores
├── src/                     # Código principal do firmware
├── platformio.ini           # Configuração do PlatformIO
├── .gitignore               # Arquivos ignorados pelo Git
└── README.md                # Documentação principal do projeto
```

## Conversão dos Sensores de Pressão

Os sensores de pressão utilizados trabalham com saída de corrente na faixa de 4–20 mA. A corrente da malha é convertida em tensão por meio de um resistor shunt de 150 ohms.

No firmware, a corrente é calculada pela relação:

```cpp
float corrente_mA = (adc_filtrado[i] / R_SHUNT) * 1000.0;
```

Em seguida, a pressão é obtida por interpolação linear entre os limites do sensor:

```cpp
pressoes_finais[i] = PRESSAO_MIN + 
                     (corrente_mA - 4.0) * 
                     (PRESSAO_MAX - PRESSAO_MIN) / 16.0;
```

Para os sensores utilizados neste projeto:

```cpp
static const float R_SHUNT = 150.0;
static const float PRESSAO_MIN = 0.5;
static const float PRESSAO_MAX = 3.0;
```

Assim, a conversão considera:

```text
4 mA  -> 0,5 bar
20 mA -> 3,0 bar
```

## Aplicação Supervisória

A aplicação localizada em `App_PC/` utiliza Python e pywebview para abrir a interface web servida pelo ESP32 em uma janela desktop.

O ESP32 disponibiliza a interface localmente, e a aplicação supervisória acessa o endereço:

```text
http://192.168.4.1
```

Além da visualização, a aplicação realiza a coleta periódica dos dados e o salvamento em arquivo CSV no computador.

## Como Executar o Firmware

1. Clone o repositório:

```bash
git clone https://github.com/eduuuxx97/esp32-pressure-sensor.git
```

2. Acesse a pasta do projeto:

```bash
cd esp32-pressure-sensor
```

3. Abra o projeto no VS Code com PlatformIO.

4. Compile o firmware:

```bash
pio run
```

5. Envie o firmware para o ESP32:

```bash
pio run --target upload
```

6. Abra o monitor serial:

```bash
pio device monitor
```

## Como Executar a Aplicação do PC

Acesse a pasta da aplicação:

```bash
cd App_PC
```

Instale as dependências necessárias:

```bash
pip install pywebview requests
```

Execute a aplicação:

```bash
python MonitorApp.py
```

## Hardware

Os arquivos do projeto da placa de circuito impresso estão disponíveis na pasta:

```text
Hardware/
```

Essa pasta contém o projeto desenvolvido no KiCad, incluindo esquemático, layout da PCB, bibliotecas de símbolos, footprints e arquivos de fabricação.

## Observações

* Os arquivos temporários, backups e ambientes virtuais não são versionados no repositório.
* O projeto utiliza conversores ADS1115 para aumentar a resolução das leituras analógicas em relação ao ADC interno do ESP32.
* A calibração experimental pode ser ajustada no firmware conforme os dados obtidos em bancada com instrumento de referência.

## Autor

Eduardo dos Santos Rodrigues
Engenharia Eletrônica — Universidade Federal de Pernambuco

GitHub: [@eduuuxx97](https://github.com/eduuuxx97)

## Repositório

Este projeto está disponível em:

```text
https://github.com/eduuuxx97/esp32-pressure-sensor
```

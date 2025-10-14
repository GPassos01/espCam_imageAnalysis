# ESP32-CAM Análise de Imagens com Edge Impulse

<div align="center">

[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](https://choosealicense.com/licenses/mit/)
[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.2+-blue.svg)](https://docs.espressif.com/projects/esp-idf/en/stable/)
[![Edge Impulse](https://img.shields.io/badge/Edge_Impulse-ML-orange.svg)](https://www.edgeimpulse.com/)
[![Contributions Welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](CONTRIBUTING.md)

*Firmware para ESP32-CAM que utiliza um modelo de Machine Learning do Edge Impulse para classificar imagens armazenadas no sistema de arquivos SPIFFS.*

</div>

---

## Índice

- [Sobre o Projeto](#-sobre-o-projeto)
- [Funcionalidades](#-funcionalidades)
- [Estrutura do Projeto](#-estrutura-do-projeto)
- [Tecnologias Utilizadas](#-tecnologias-utilizadas)
- [Início Rápido](#-início-rápido)
- [Como Contribuir](#-como-contribuir)
- [Licença](#-licença)

## Sobre o Projeto

Este projeto implementa um firmware para a placa **ESP32-CAM** capaz de executar inferências de Machine Learning em imagens. O sistema é projetado para carregar imagens diretamente do sistema de arquivos flash (SPIFFS) e classificá-las usando um modelo treinado na plataforma **Edge Impulse**.

É uma solução embarcada para tarefas de visão computacional, ideal para aplicações de baixo custo e baixo consumo de energia onde a análise de imagem precisa ocorrer diretamente no dispositivo.

### Aplicações
- **Classificação de imagens** em campo.
- **Monitoramento autônomo** de ambientes.
- **Prova de conceito** para sistemas de visão embarcados.
- **Análise de datasets** de imagens diretamente no hardware de destino.

## Funcionalidades

- [x] **Firmware baseado em ESP-IDF**: Utiliza o framework oficial da Espressif para máxima performance e compatibilidade.
- [x] **Integração com Edge Impulse**: Executa modelos de classificação de imagem (TFLite for Microcontrollers) de forma otimizada.
- [x] **Processamento via SPIFFS**: Carrega e processa um ou mais arquivos de imagem `.jpg` armazenados na partição SPIFFS.
- [x] **Pré-processamento Portátil**: Inclui um módulo de pré-processamento de imagem que garante consistência entre o teste no PC e a execução no dispositivo.
- [x] **Ferramenta de Validação**: Acompanha um CLI (`host_infer`) que permite testar o modelo e o pré-processamento no PC antes de embarcar.
- [x] **Conectividade**: Inclui suporte a WiFi e MQTT para envio dos resultados da classificação.
- [x] **Gerenciamento de Memória**: Otimizado para usar a PSRAM externa, permitindo o processamento de imagens maiores.

## Estrutura do Projeto

A estrutura do projeto foi simplificada para focar nos componentes essenciais: o firmware e as ferramentas de validação.

```
espCam_imageAnalysis/
├── src/
│   └── firmware/             # Código fonte do firmware (ESP-IDF)
│       ├── main/             # Lógica principal da aplicação
│       │   ├── components/   # Módulos de software
│       │   │   ├── analysis_manager/
│       │   │   ├── network_manager/
│       │   │   └── spiffs_manager/
│       │   ├── model/        # Código de inferência do Edge Impulse
│       │   └── portable/     # Módulo de pré-processamento
│       ├── data/             # Imagens para serem gravadas no SPIFFS
│       └── partitions.csv    # Tabela de partição (app + spiffs)
├── archive/                  # Dataset completo de imagens para teste
├── tools/
│   ├── host_infer/           # Ferramenta para testar o modelo no PC
│   └── ...                   # Scripts para build e teste
├── README.md                 # Este arquivo
└── LICENSE                   # Licença MIT
```

## Tecnologias Utilizadas

### Firmware (ESP32-CAM)
- **ESP-IDF v5.2+**: Framework oficial da Espressif.
- **FreeRTOS**: Sistema operacional de tempo real.
- **Edge Impulse SDK**: Para execução do modelo de ML.
- **TFLite for Microcontrollers**: Backend de inferência.
- **MQTT**: Protocolo para envio de resultados.

### Ferramentas de Desenvolvimento
- **CMake**: Sistema de build.
- **ImageMagick**: Para conversão de imagens para o teste no host.
- **Git**: Controle de versão.

## Início Rápido

### Pré-requisitos

- **Hardware**:
  - ESP32-CAM (AI-Thinker ou similar)
  - Conversor FTDI para programação
- **Software**:
  - ESP-IDF v5.2 ou superior configurado.
  - As dependências da ferramenta `host_infer` (como `gcc`, `g++`, `cmake`).
  - ImageMagick (`convert`).

### Testando no PC (Host)

É altamente recomendado validar o modelo no PC antes de embarcar.

```bash
# 1. Compile a ferramenta de teste
cd tools/host_infer
mkdir build && cd build
cmake ..
make

# 2. Execute o script de teste em todo o dataset
cd ../../..  # Voltar para a raiz do projeto
./tools/test_dataset_summary.sh
```

### Compilando e Gravando o Firmware

```bash
# 1. Adicione a imagem que deseja testar na ESP32
#    - Coloque um arquivo .jpg dentro de 'src/firmware/data/'

# 2. Navegue para o diretório do firmware
cd src/firmware

# 3. Configure o ESP-IDF (se ainda não o fez)
. $IDF_PATH/export.sh

# 4. Compile, grave o firmware e a partição SPIFFS
idf.py -p /dev/ttyUSB0 build flash storage-flash monitor
```
> **Nota**: Substitua `/dev/ttyUSB0` pela porta serial correta do seu dispositivo.

## Como Contribuir

Contribuições são bem-vindas! Por favor, leia nosso [Guia de Contribuição](CONTRIBUTING.md) para mais detalhes.

## Licença

Distribuído sob a Licença MIT. Veja `LICENSE` para mais informações.

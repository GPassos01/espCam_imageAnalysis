# Guia de Instalação

Este guia fornece instruções detalhadas para instalar e configurar o ESP32-CAM Flood Monitor em diferentes ambientes.

## Índice

- [Pré-requisitos](#-pré-requisitos)
- [Instalação do ESP-IDF](#-instalação-do-esp-idf)
- [Setup do Hardware](#-setup-do-hardware)
- [Configuração do Firmware](#-configuração-do-firmware)
- [Instalação do Servidor](#-instalação-do-servidor)
- [Verificação da Instalação](#-verificação-da-instalação)
- [Troubleshooting](#-troubleshooting)

## Pré-requisitos

### Hardware Necessário

| Item | Especificação | Observações |
|------|---------------|-------------|
| **ESP32-CAM** | AI-Thinker ou similar | Com 8MB PSRAM recomendado |
| **Programador FTDI** | 3.3V/5V | Para upload do firmware |
| **Fonte de Alimentação** | 5V/3A | Externa para operação estável |
| **Cartão MicroSD** | Classe 10, 32GB+ | Opcional para armazenamento local |
| **Jumpers** | Conexão entre pinos | Para modo de programação |

### Software Necessário

#### Sistema Operacional
- **Linux:** Ubuntu 20.04+ (recomendado)
- **macOS:** 10.15+ (Catalina)
- **Windows:** 10/11 com WSL2

#### Ferramentas
- **Git 2.25+**
- **Python 3.9+**
- **ESP-IDF v5.0+**
- **Editor de código** (VS Code recomendado)

## Instalação do ESP-IDF

### Linux/macOS

```bash
# 1. Instalar dependências
sudo apt update && sudo apt install -y \
    git wget flex bison gperf python3-pip python3-venv \
    cmake ninja-build ccache libffi-dev libssl-dev \
    dfu-util libusb-1.0-0

# 2. Baixar ESP-IDF
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout v5.0.1

# 3. Instalar ferramentas
./install.sh esp32

# 4. Configurar ambiente
source export.sh

# 5. Adicionar ao bashrc (opcional)
echo 'alias get_idf=". $HOME/esp/esp-idf/export.sh"' >> ~/.bashrc
```

### Windows (WSL2)

```bash
# 1. Instalar WSL2 Ubuntu
wsl --install -d Ubuntu-22.04

# 2. No Ubuntu WSL, seguir os mesmos passos do Linux
# Certificar-se de que o USB está passthrough configurado
```

### Verificação da Instalação

```bash
# Testar se ESP-IDF está funcionando
idf.py --version
# Deve retornar: ESP-IDF v5.0.1

# Verificar toolchain
xtensa-esp32-elf-gcc --version
```

## Setup do Hardware

### Conexões ESP32-CAM + FTDI

| ESP32-CAM | FTDI | Função |
|-----------|------|--------|
| VCC | 5V | Alimentação |
| GND | GND | Terra |
| U0T | RX | Transmissão |
| U0R | TX | Recepção |
| IO0 | GND | Modo Programming* |

**\*Conectar IO0 ao GND apenas durante o upload do firmware**

### Esquema de Conexão

```
ESP32-CAM          FTDI Programmer
┌─────────┐       ┌─────────────┐
│   VCC   ├───────┤     5V      │
│   GND   ├───────┤    GND      │
│   U0T   ├───────┤     RX      │
│   U0R   ├───────┤     TX      │
│   IO0   ├───┐   │             │
└─────────┘   │   └─────────────┘
              │
              └─── GND (só para upload)
```

## Configuração do Firmware

### 1. Clone do Projeto

```bash
# Clone o repositório
git clone https://github.com/seu-usuario/esp32-cam-flood-monitor.git
cd esp32-cam-flood-monitor

# Verificar estrutura
ls -la
# Deve mostrar: esp32/, server/, docs/, etc.
```

### 2. Configuração WiFi/MQTT

```bash
# Navegar para o diretório ESP32
cd esp32

# Copiar template de configuração
cp main/config.example.h main/config.h

# Editar configurações
nano main/config.h
```

**Configurações Principais (`config.h`):**

```c
// WiFi Configuration
#define WIFI_SSID "SUA_REDE_WIFI"
#define WIFI_PASSWORD "SUA_SENHA_WIFI"

// MQTT Configuration  
#define MQTT_BROKER_URI "mqtt://SEU_BROKER_MQTT:1883"
#define MQTT_USERNAME "seu_usuario"
#define MQTT_PASSWORD "sua_senha"

// System Configuration
#define DEVICE_ID "ESP32CAM_001"
#define LOCATION_NAME "Rio_Principal_Sensor01"
```

### 3. Seleção da Versão

```bash
# Versão Inteligente (recomendada)
echo "INTELLIGENT" > main/ACTIVE_VERSION.txt

# OU Versão Simples (baseline)
echo "SIMPLE" > main/ACTIVE_VERSION.txt
```

### 4. Build e Upload

```bash
# Configurar projeto
idf.py menuconfig
# (usar configurações padrão para a maioria)

# Build do firmware
idf.py build

# Conectar ESP32-CAM com IO0 ligado ao GND
# Upload do firmware
idf.py -p /dev/ttyUSB0 flash

# Remover conexão IO0-GND e resetar
# Monitorar saída
idf.py -p /dev/ttyUSB0 monitor
```

## Instalação do Servidor

### 1. Ambiente Python

```bash
# Navegar para servidor
cd server/

# Criar ambiente virtual
python3 -m venv esp32_monitor_env
source esp32_monitor_env/bin/activate

# Instalar dependências
pip install --upgrade pip
pip install -r requirements.txt
```

### 2. Configuração MQTT Broker

#### Instalação Mosquitto (Ubuntu)

```bash
# Instalar mosquitto
sudo apt install mosquitto mosquitto-clients

# Configurar
sudo nano /etc/mosquitto/mosquitto.conf
```

**Configuração básica (`mosquitto.conf`):**

```conf
# Porta padrão
port 1883

# Permitir conexões anônimas (desenvolvimento)
allow_anonymous true

# Log
log_dest file /var/log/mosquitto/mosquitto.log
log_type all

# Persistence
persistence true
persistence_location /var/lib/mosquitto/
```

```bash
# Iniciar serviço
sudo systemctl enable mosquitto
sudo systemctl start mosquitto

# Verificar status
sudo systemctl status mosquitto
```

### 3. Teste do Servidor

```bash
# Executar coletor de dados
python mqtt_data_collector.py

# Em outro terminal, testar MQTT
mosquitto_pub -h localhost -t "test/topic" -m "Hello MQTT"
```

## Verificação da Instalação

### Checklist Completo

#### Hardware
- [ ] ESP32-CAM conectado corretamente
- [ ] FTDI funcionando (device `/dev/ttyUSB0` visível)
- [ ] Fonte de alimentação adequada
- [ ] LEDs do ESP32-CAM piscando

#### Software
- [ ] ESP-IDF instalado e funcionando
- [ ] Projeto compila sem erros (`idf.py build`)
- [ ] Upload realizado com sucesso
- [ ] Monitor serial mostra logs do sistema

#### Rede
- [ ] ESP32-CAM conecta ao WiFi
- [ ] MQTT broker acessível
- [ ] Servidor Python recebe dados

#### Funcionamento
- [ ] ESP32-CAM captura imagens
- [ ] Dados são transmitidos via MQTT
- [ ] Servidor armazena no banco SQLite
- [ ] Logs não mostram erros críticos

### Comandos de Verificação

```bash
# 1. Verificar ESP-IDF
idf.py --version

# 2. Verificar conexão serial
ls /dev/ttyUSB*

# 3. Verificar MQTT broker
mosquitto_sub -h localhost -t "+/+"

# 4. Verificar Python
python --version
pip list | grep paho-mqtt

# 5. Verificar banco de dados
ls -la server/monitoring_data.db
```

## Troubleshooting

### Problemas Comuns

#### ESP32-CAM não conecta

**Sintoma:** Erro de conexão serial
```bash
# Soluções
sudo usermod -a -G dialout $USER  # Adicionar usuário ao grupo
sudo chmod 666 /dev/ttyUSB0       # Permissões temporárias
# Logout/login para aplicar mudanças
```

#### WiFi não conecta

**Sintoma:** `WIFI_EVENT_STA_DISCONNECTED`
```c
// Verificar configurações em config.h
#define WIFI_SSID "NOME_CORRETO"
#define WIFI_PASSWORD "SENHA_CORRETA"

// Verificar tipo de segurança (WPA2/WPA3)
```

#### MQTT falha

**Sintoma:** `MQTT_EVENT_DISCONNECTED`
```bash
# Verificar broker
sudo systemctl status mosquitto

# Testar conexão manual
mosquitto_pub -h localhost -t "test" -m "teste"

# Verificar firewall
sudo ufw status
```

#### Erro de memória

**Sintoma:** `ESP_ERR_NO_MEM`
```c
// Verificar PSRAM em menuconfig
Component Config → ESP32-specific → Support for external SPI RAM
```

#### Build falha

**Sintoma:** Erros de compilação
```bash
# Limpar build
idf.py fullclean

# Verificar dependências
git submodule update --init --recursive

# Recompilar
idf.py build
```

### Logs Úteis

```bash
# Logs do ESP32
idf.py monitor

# Logs do MQTT broker
sudo tail -f /var/log/mosquitto/mosquitto.log

# Logs do servidor Python
tail -f server/logs/mqtt_collector.log
```

### Suporte Adicional

- 📖 [Documentação Oficial ESP-IDF](https://docs.espressif.com/projects/esp-idf/)
- 🔧 [Troubleshooting Detalhado](troubleshooting.md)
- 💬 [Issues do GitHub](https://github.com/seu-usuario/esp32-cam-flood-monitor/issues)
- 📧 [Contato Direto](mailto:gabriel.passos@unesp.br)

---

**Próximos Passos:** [Configuração Avançada](configuration.md) | [Guia de Uso](usage.md) 

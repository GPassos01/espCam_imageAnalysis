# 📦 Guia de Instalação - Sistema de Monitoramento ESP32-CAM

Este guia detalha o processo completo de instalação e configuração do sistema.

## 📋 Índice

1. [Pré-requisitos](#pré-requisitos)
2. [Instalação ESP-IDF](#instalação-esp-idf)
3. [Configuração do Projeto](#configuração-do-projeto)
4. [Compilação e Deploy](#compilação-e-deploy)
5. [Servidor Python](#servidor-python)
6. [Verificação](#verificação)

---

## 🔧 Pré-requisitos

### Software
- Sistema operacional: Linux, macOS ou Windows (WSL)
- Git
- Python 3.8+ com pip
- Ferramentas de build (make, cmake, ninja)

### Hardware
- ESP32-CAM AI-Thinker
- Adaptador FTDI USB-Serial
- Fonte de alimentação 5V/2A
- Jumpers para conexão

---

## 🛠️ Instalação ESP-IDF

### Linux/macOS

```bash
# 1. Criar diretório para ESP-IDF
mkdir -p ~/esp
cd ~/esp

# 2. Clonar ESP-IDF v5.0
git clone -b v5.0 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf

# 3. Instalar ferramentas
./install.sh esp32

# 4. Configurar ambiente (executar sempre antes de usar)
. ./export.sh

# 5. Verificar instalação
idf.py --version
```

### Windows

Use o instalador oficial ou WSL2:
- [ESP-IDF Tools Installer](https://dl.espressif.com/dl/esp-idf-tools-setup-online-5.0.exe)

### Componente ESP32-Camera

```bash
cd $IDF_PATH/components
git clone https://github.com/espressif/esp32-camera.git
```

---

## ⚙️ Configuração do Projeto

### 1. Clone o Repositório

```bash
cd ~/projetos  # ou seu diretório de projetos
git clone https://github.com/usuario/ESP32-IC_Project.git
cd ESP32-IC_Project
```

### 2. Configurar Credenciais

```bash
# Editar arquivo de configuração
nano esp32/main/config.h
```

Altere as seguintes linhas:

```c
// WiFi - IMPORTANTE: Use rede 2.4GHz
#define WIFI_SSID        "Nome_Da_Sua_Rede"
#define WIFI_PASS        "Senha_Da_Rede"

// MQTT Broker
#define MQTT_BROKER_URI  "mqtt://192.168.1.100:1883"  // IP do seu broker
#define MQTT_USERNAME    ""  // Se necessário
#define MQTT_PASSWORD    ""  // Se necessário

// Identificação do dispositivo
#define DEVICE_ID        "esp32_cam_001"
```

### 3. Ajustar Parâmetros (Opcional)

```c
// Intervalos de operação
#define CAPTURE_INTERVAL_MS     15000    // 15 segundos entre capturas
#define STATUS_INTERVAL_MS      300000   // 5 minutos para estatísticas

// Thresholds de detecção
#define CHANGE_THRESHOLD       1.0f      // 1% - mudança mínima
#define ALERT_THRESHOLD        8.0f      // 8% - alerta

// WiFi Sniffer
#define SNIFFER_ENABLED        true      // Habilitar monitoramento
#define SNIFFER_CHANNEL        0         // 0 = automático
```

---

## 🔨 Compilação e Deploy

### Usando Script Automatizado

```bash
cd scripts
./setup.sh

# Menu interativo:
# 1) Verificar dependências
# 2) Configurar projeto
# 5) Compilar firmware
# 6) Flash ESP32-CAM
```

### Compilação Manual

```bash
cd esp32

# Configurar target
idf.py set-target esp32

# Compilar
idf.py build

# Verificar tamanho
idf.py size
```

### Flash do Firmware

#### Conexões FTDI

```
ESP32-CAM    FTDI
---------    ----
5V      ───  5V
GND     ───  GND
U0R     ───  TX
U0T     ───  RX
IO0     ───  GND (apenas durante upload)
```

#### Procedimento

```bash
# 1. Conectar IO0 ao GND
# 2. Conectar FTDI ao USB
# 3. Resetar ESP32-CAM

# Flash
idf.py -p /dev/ttyUSB0 flash

# 4. Remover jumper IO0-GND
# 5. Resetar ESP32-CAM

# Monitor serial
idf.py -p /dev/ttyUSB0 monitor
```

**Portas comuns:**
- Linux: `/dev/ttyUSB0` ou `/dev/ttyACM0`
- macOS: `/dev/cu.usbserial-*`
- Windows: `COM3`, `COM4`, etc.

---

## 🐍 Servidor Python

### 1. Instalar Broker MQTT

#### Linux (Debian/Ubuntu)
```bash
sudo apt update
sudo apt install mosquitto mosquitto-clients
sudo systemctl start mosquitto
```

#### macOS
```bash
brew install mosquitto
brew services start mosquitto
```

#### Windows
Baixe de: https://mosquitto.org/download/

### 2. Configurar Ambiente Python

```bash
cd server

# Criar ambiente virtual
python3 -m venv venv

# Ativar ambiente
source venv/bin/activate  # Linux/macOS
# ou
venv\Scripts\activate     # Windows

# Instalar dependências
pip install --upgrade pip
pip install -r requirements_ic.txt
```

### 3. Configurar Monitor

Edite `server/ic_monitor.py` se necessário:

```python
# Configurações MQTT
MQTT_BROKER = "192.168.1.100"  # IP do broker
MQTT_PORT = 1883
```

### 4. Executar Monitor

```bash
# Com ambiente virtual ativado
python3 ic_monitor.py
```

---

## ✅ Verificação

### 1. Verificar ESP32-CAM

No monitor serial, você deve ver:

```
🔍 Sistema de Monitoramento por Imagens
📷 Inicializando câmera...
🌐 Conectando WiFi...
📡 Conectando MQTT...
✅ Sistema de monitoramento iniciado!
```

### 2. Verificar MQTT

```bash
# Testar conexão
mosquitto_sub -h localhost -t '#' -v

# Você deve ver mensagens como:
monitoring/data {"timestamp":1234567890,"device":"esp32_cam_001"...}
```

### 3. Verificar Servidor Python

```
🚀 Iniciando Sistema de Monitoramento de Imagens
📊 Banco de dados configurado com sucesso
🌐 Conectado ao broker MQTT
📡 Inscrito em: monitoring/data
📡 Aguardando dados via MQTT...
```

---

## 🚨 Solução de Problemas

### ESP32-CAM não conecta ao WiFi

1. Verificar se a rede é 2.4GHz (não funciona em 5GHz)
2. Confirmar SSID e senha
3. Verificar alcance do sinal
4. Tentar com outra rede

### Erro "Camera probe failed"

1. Verificar alimentação (use fonte externa 5V/2A)
2. Confirmar que PSRAM está habilitado no sdkconfig
3. Testar com outra ESP32-CAM

### MQTT não conecta

1. Verificar se broker está rodando: `sudo systemctl status mosquitto`
2. Testar conexão: `mosquitto_pub -h localhost -t test -m "hello"`
3. Verificar firewall/portas

### Python - ModuleNotFoundError

```bash
# Certificar que ambiente virtual está ativo
which python  # Deve mostrar caminho do venv

# Reinstalar dependências
pip install -r requirements_ic.txt
```

---

## 📝 Próximos Passos

1. Verificar logs em tempo real
2. Gerar primeiro relatório com `scripts/generate_report.py`
3. Ajustar thresholds conforme necessário
4. Configurar backup automático do banco de dados

Para mais detalhes técnicos, consulte a [Documentação Técnica](DOCUMENTACAO_TECNICA.md). 
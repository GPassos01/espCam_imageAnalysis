# 🚀 Setup Básico - ESP32-CAM Flood Monitor

Este exemplo mostra como configurar o sistema de forma simples para começar a monitorar.

## 📋 O que você vai aprender

- Configuração básica do ESP32-CAM
- Setup do servidor Python local
- Primeira captura de imagens
- Monitoramento básico via MQTT

## 🔧 Pré-requisitos

- ESP32-CAM AI-Thinker
- Cabo FTDI para programação
- Python 3.9+
- Broker MQTT (Mosquitto)

## 🎯 Configuração Rápida

### 1. Clone e Configure

```bash
# Clone o projeto
git clone https://github.com/seu-usuario/esp32-cam-flood-monitor.git
cd esp32-cam-flood-monitor

# Configure ESP32
cd src/firmware
cp main/config.example.h main/config.h
```

### 2. Editar Configurações

Edite `src/firmware/main/config.h`:

```c
// WiFi (OBRIGATÓRIO)
#define WIFI_SSID "SUA_REDE_2.4GHZ"
#define WIFI_PASSWORD "SUA_SENHA"

// MQTT (OBRIGATÓRIO)  
#define MQTT_BROKER_URI "mqtt://192.168.1.100:1883"
#define DEVICE_ID "ESP32CAM_TESTE"
```

### 3. Versão Simples (Recomendada para início)

```bash
# Selecionar versão simples
echo "SIMPLE" > main/ACTIVE_VERSION.txt
```

### 4. Compilar e Enviar

```bash
# Build
idf.py build

# Upload (conectar IO0 ao GND primeiro)
idf.py -p /dev/ttyUSB0 flash monitor
```

### 5. Configurar Servidor

```bash
# Terminal separado
cd src/server

# Ambiente virtual
python3 -m venv venv
source venv/bin/activate

# Instalar dependências
pip install -r requirements.txt

# Executar
python mqtt_data_collector.py
```

## ✅ Verificação

### No Monitor Serial (ESP32):
```
🌐 WiFi conectado: 192.168.1.150
📡 MQTT conectado: localhost:1883
📷 Captura iniciada a cada 15s
```

### No Servidor Python:
```
🚀 Sistema iniciado
📡 Conectado ao broker MQTT
📊 Dados recebidos: ESP32CAM_TESTE
```

## 📊 Primeiros Dados

O sistema vai:
1. ✅ Capturar imagem a cada 15 segundos
2. ✅ Enviar via MQTT  
3. ✅ Armazenar no banco SQLite
4. ✅ Salvar imagens em `received_images/`

## 🔍 Monitoramento

```bash
# Ver dados em tempo real
tail -f logs/monitor.log

# Verificar banco de dados
sqlite3 monitoring_data.db "SELECT * FROM monitoring_data ORDER BY timestamp DESC LIMIT 5;"
```

## 🎯 Próximos Passos

1. 📖 [Configuração Avançada](../advanced_config/)
2. 📊 [Análise Científica](../scientific_analysis/)
3. 🔧 [Troubleshooting](../../docs/troubleshooting.md)

## ❓ Problemas Comuns

### ESP32 não conecta WiFi
- Verificar se é rede 2.4GHz
- Confirmar SSID e senha
- Tentar com hotspot do celular

### MQTT falha
```bash
# Testar broker
mosquitto_pub -h localhost -t "test" -m "hello"
```

### Imagens não aparecem
- Verificar source de alimentação (use 5V externa)
- Confirmar que PSRAM está habilitado

---

> 💡 **Dica:** Comece sempre com a versão SIMPLES, depois passe para INTELLIGENT quando estiver familiarizado! 
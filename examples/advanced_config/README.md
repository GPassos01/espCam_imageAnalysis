# ⚡ Configuração Avançada - ESP32-CAM Flood Monitor

Este exemplo mostra configurações avançadas para maximizar a performance e funcionalidades do sistema.

## 🎯 O que você vai aprender

- Configuração da versão INTELLIGENT
- Otimização de parâmetros de detecção
- Setup de produção com TLS/SSL
- Monitoramento de múltiplos dispositivos
- Análise de performance em tempo real

## 🔧 Pré-requisitos

- Conhecimento básico do sistema
- ESP32-CAM com 8MB PSRAM
- Broker MQTT profissional (HiveMQ/AWS IoT)
- Sistema Linux para produção

## ⚙️ Configuração da Versão Inteligente

### 1. Ativar Versão INTELLIGENT

```bash
cd src/firmware
echo "INTELLIGENT" > main/ACTIVE_VERSION.txt
```

### 2. Configurações Avançadas

Edite `main/config.h`:

```c
// === CONFIGURAÇÕES AVANÇADAS ===

// Análise Inteligente
#define ENABLE_ADVANCED_ANALYSIS    true
#define CHANGE_THRESHOLD            3.0f     // 3% mudança mínima
#define ALERT_THRESHOLD             12.0f    // 12% para alertas
#define NOISE_FILTER_ENABLED        true

// Performance Otimizada
#define CAPTURE_INTERVAL_MS         10000    // 10 segundos
#define JPEG_QUALITY               5        // Premium quality
#define FRAMESIZE                  FRAMESIZE_HVGA  // 480x320

// Buffer Histórico
#define HISTORY_BUFFER_SIZE        3        // 3 imagens de referência
#define REFERENCE_UPDATE_INTERVAL  300000   // 5 minutos

// Sistema Anti-esverdeado
#define GREEN_DETECTION_ENABLED    true
#define GREEN_CORRECTION_RETRIES   3
#define WARMUP_CAPTURES           2

// Monitoramento Avançado
#define ENABLE_WIFI_SNIFFER        true
#define MEMORY_MONITORING          true
#define PERFORMANCE_METRICS        true

// Segurança
#define MQTT_TLS_ENABLED           true
#define DEVICE_ID                  "ESP32CAM_PROD_001"
#define LOCATION_NAME              "Rio_Principal_Montante"
```

### 3. Configuração MQTT Segura

```c
// MQTT com TLS
#define MQTT_BROKER_URI    "mqtts://seu-broker-seguro.com:8883"
#define MQTT_CA_CERT       "-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----"
#define MQTT_USERNAME      "esp32_device_001"
#define MQTT_PASSWORD      "senha_super_segura"

// Tópicos específicos
#define MQTT_TOPIC_DATA    "flood_monitor/sensors/001/data"
#define MQTT_TOPIC_IMAGES  "flood_monitor/sensors/001/images"
#define MQTT_TOPIC_ALERTS  "flood_monitor/sensors/001/alerts"
```

## 📊 Servidor Python Avançado

### 1. Configuração de Produção

```python
# config.py
import os

# MQTT Seguro
MQTT_BROKER = os.getenv('MQTT_BROKER', 'production-broker.com')
MQTT_PORT = 8883
MQTT_TLS_ENABLED = True
MQTT_CA_CERT_PATH = '/etc/ssl/certs/mqtt-ca.crt'
MQTT_CLIENT_CERT_PATH = '/etc/ssl/certs/mqtt-client.crt'
MQTT_CLIENT_KEY_PATH = '/etc/ssl/private/mqtt-client.key'

# Database de Produção
DATABASE_URL = 'postgresql://user:pass@localhost/esp32_monitor'
# ou manter SQLite para projetos menores
DATABASE_PATH = '/var/lib/esp32monitor/monitoring_data.db'

# Performance
WORKER_THREADS = 4
BUFFER_SIZE = 1000
BATCH_INSERT_SIZE = 50

# Alertas
EMAIL_ALERTS_ENABLED = True
SMTP_SERVER = 'smtp.gmail.com'
ALERT_RECIPIENTS = ['admin@empresa.com', 'ops@empresa.com']

# API REST
API_ENABLED = True
API_PORT = 8080
API_AUTH_TOKEN = 'token_super_secreto'

# Backup Automático
BACKUP_ENABLED = True
BACKUP_INTERVAL = 86400  # 24 horas
BACKUP_RETENTION_DAYS = 30
S3_BACKUP_BUCKET = 'esp32-monitor-backups'
```

### 2. Múltiplos Dispositivos

```python
# multi_device_config.py
DEVICES = {
    'ESP32CAM_001': {
        'location': 'Rio Principal - Montante',
        'coordinates': (-22.4186, -47.5647),
        'alert_threshold': 12.0,
        'priority': 'high'
    },
    'ESP32CAM_002': {
        'location': 'Rio Principal - Jusante', 
        'coordinates': (-22.4200, -47.5660),
        'alert_threshold': 15.0,
        'priority': 'medium'
    },
    'ESP32CAM_003': {
        'location': 'Afluente Norte',
        'coordinates': (-22.4150, -47.5600),
        'alert_threshold': 8.0,
        'priority': 'high'
    }
}

# Tópicos dinâmicos
MQTT_TOPICS = [
    f"flood_monitor/sensors/{device_id}/+"
    for device_id in DEVICES.keys()
]
```

## 🔍 Monitoramento Avançado

### 1. Dashboard Web

```bash
# Instalar dependências extras
pip install flask plotly dash redis

# Executar dashboard
python src/server/web/dashboard.py
```

Acesse: `http://servidor:8080`

### 2. Métricas em Tempo Real

```python
# metrics_collector.py
import psutil
import time
from datetime import datetime

def collect_system_metrics():
    return {
        'timestamp': datetime.now().isoformat(),
        'cpu_percent': psutil.cpu_percent(),
        'memory_percent': psutil.virtual_memory().percent,
        'disk_usage': psutil.disk_usage('/').percent,
        'network_io': psutil.net_io_counters()._asdict(),
        'mqtt_connections': get_mqtt_connection_count(),
        'database_size': get_database_size(),
        'active_devices': get_active_device_count()
    }
```

### 3. Alertas Inteligentes

```python
# alert_system.py
class AlertSystem:
    def __init__(self):
        self.rules = [
            # Alerta de enchente iminente
            {
                'condition': 'change_percentage > 15 AND last_5min_avg > 10',
                'severity': 'critical',
                'action': 'send_sms_and_email'
            },
            # Dispositivo offline
            {
                'condition': 'last_seen > 300',  # 5 minutos
                'severity': 'warning', 
                'action': 'send_email'
            },
            # Análise de tendência
            {
                'condition': 'trend_1hour > 5 AND weather_forecast == rain',
                'severity': 'warning',
                'action': 'increase_monitoring_frequency'
            }
        ]
```

## 🚀 Deploy de Produção

### 1. Docker Compose

```yaml
# docker-compose.yml
version: '3.8'
services:
  mqtt-broker:
    image: eclipse-mosquitto:2.0
    ports:
      - "1883:1883"
      - "8883:8883"
    volumes:
      - ./config/mosquitto.conf:/mosquitto/config/mosquitto.conf
      - ./certs:/mosquitto/certs
      
  esp32-server:
    build: .
    ports:
      - "8080:8080"
    environment:
      - MQTT_BROKER=mqtt-broker
      - DATABASE_PATH=/data/monitoring.db
    volumes:
      - ./data:/data
      - ./logs:/app/logs
    depends_on:
      - mqtt-broker
      
  redis:
    image: redis:7-alpine
    ports:
      - "6379:6379"
      
  grafana:
    image: grafana/grafana:latest
    ports:
      - "3000:3000"
    environment:
      - GF_SECURITY_ADMIN_PASSWORD=admin123
    volumes:
      - grafana-storage:/var/lib/grafana

volumes:
  grafana-storage:
```

### 2. Systemd Service

```bash
# /etc/systemd/system/esp32-monitor.service
[Unit]
Description=ESP32-CAM Flood Monitor
After=network.target mosquitto.service

[Service]
Type=simple
User=esp32monitor
Group=esp32monitor
WorkingDirectory=/opt/esp32-monitor
ExecStart=/opt/esp32-monitor/venv/bin/python src/server/mqtt_data_collector.py
Restart=always
RestartSec=10
Environment=PYTHONPATH=/opt/esp32-monitor

# Limites de recursos
MemoryMax=512M
CPUQuota=200%

[Install]
WantedBy=multi-user.target
```

## 📈 Otimização de Performance

### 1. Configurações ESP32

```c
// sdkconfig customizado
CONFIG_ESP32_DEFAULT_CPU_FREQ_240=y
CONFIG_ESP32_SPIRAM_SUPPORT=y
CONFIG_SPIRAM_USE_CAPS_ALLOC=y
CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL=16384

// Otimizações de WiFi
CONFIG_ESP32_WIFI_DYNAMIC_RX_BUFFER_NUM=16
CONFIG_ESP32_WIFI_DYNAMIC_TX_BUFFER_NUM=16
CONFIG_ESP32_WIFI_AMPDU_TX_ENABLED=y
CONFIG_ESP32_WIFI_AMPDU_RX_ENABLED=y
```

### 2. Tuning do Servidor

```python
# performance_config.py

# Otimizações MQTT
MQTT_KEEPALIVE = 60
MQTT_QOS = 1
MQTT_CLEAN_SESSION = False

# Pool de conexões
CONNECTION_POOL_SIZE = 10
CONNECTION_POOL_TIMEOUT = 30

# Cache Redis
REDIS_CACHE_TTL = 300  # 5 minutos
REDIS_MAX_CONNECTIONS = 20

# Processamento assíncrono
ASYNC_PROCESSING = True
WORKER_QUEUE_SIZE = 1000
```

## 🧪 Testes de Performance

```bash
# Teste de carga MQTT
tools/analysis/run_performance_tests.sh

# Benchmarks de detecção
python tools/analysis/benchmark_detection.py

# Teste de múltiplos dispositivos
python tools/analysis/multi_device_test.py --devices 10 --duration 3600
```

## 📊 Métricas de Sucesso

- **Latência MQTT:** < 100ms
- **Detecção de mudanças:** > 95% precisão
- **Uptime:** > 99.5%
- **Consumo de memória:** < 70% PSRAM
- **Throughput:** > 100 imagens/hora por device

---

> ⚡ **Aviso:** Esta configuração é para ambientes de produção. Teste primeiro em ambiente de desenvolvimento! 
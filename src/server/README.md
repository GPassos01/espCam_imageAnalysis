# 🐍 ESP32-CAM Flood Monitor - Servidor Python

Servidor de coleta e análise de dados para o sistema de monitoramento fluvial ESP32-CAM.

## 📋 Visão Geral

Este módulo Python é responsável por:
- 📡 **Recepção de dados MQTT** do ESP32-CAM
- 💾 **Armazenamento em SQLite** para análise posterior
- 📊 **Processamento e análise** dos dados coletados
- 🌐 **Interface web** para monitoramento em tempo real
- 📈 **Geração de relatórios** científicos

## 🚀 Início Rápido

### Pré-requisitos

```bash
# Python 3.9+ necessário
python --version

# Dependências do sistema (Ubuntu/Debian)
sudo apt install python3-pip python3-venv mosquitto mosquitto-clients
```

### Instalação Rápida

```bash
# 1. Navegar para o diretório do servidor
cd server/

# 2. Criar ambiente virtual
python3 -m venv esp32_monitor_env
source esp32_monitor_env/bin/activate

# 3. Instalar dependências
pip install --upgrade pip
pip install -r requirements.txt

# 4. Configurar MQTT broker local
sudo systemctl start mosquitto

# 5. Executar servidor
python mqtt_data_collector.py
```

## 📁 Estrutura do Projeto

```
server/
├── mqtt_data_collector.py      # 🚀 Script principal de coleta
├── requirements.txt            # 📦 Dependências Python
├── config.py                   # ⚙️ Configurações do servidor
├── database/                   # 💾 Módulos de banco de dados
│   ├── __init__.py
│   ├── models.py              # 📊 Modelos de dados
│   └── connection.py          # 🔗 Conexão SQLite
├── mqtt/                       # 📡 Módulos MQTT
│   ├── __init__.py
│   ├── client.py              # 📱 Cliente MQTT
│   └── handlers.py            # 🔄 Handlers de mensagens
├── analysis/                   # 📈 Análise de dados
│   ├── __init__.py
│   ├── image_processor.py     # 🖼️ Processamento de imagens
│   └── statistics.py          # 📊 Análise estatística
├── web/                        # 🌐 Interface web
│   ├── app.py                 # 🖥️ Aplicação Flask
│   ├── templates/             # 📄 Templates HTML
│   └── static/                # 🎨 CSS/JS/Imagens
├── logs/                       # 📋 Logs do sistema
├── tests/                      # 🧪 Testes unitários
├── scripts/                    # 🛠️ Scripts utilitários
└── monitoring_data.db          # 💾 Banco de dados SQLite
```

## ⚙️ Configuração

### Arquivo de Configuração

Edite `config.py` para suas necessidades:

```python
# config.py
import os

# Configurações MQTT
MQTT_BROKER = os.getenv('MQTT_BROKER', 'localhost')
MQTT_PORT = int(os.getenv('MQTT_PORT', 1883))
MQTT_USERNAME = os.getenv('MQTT_USERNAME', '')
MQTT_PASSWORD = os.getenv('MQTT_PASSWORD', '')

# Tópicos MQTT
TOPICS = {
    'data': 'monitoring/data',
    'images': 'monitoring/images',
    'status': 'monitoring/status',
    'alerts': 'monitoring/alerts'
}

# Banco de Dados
DATABASE_PATH = 'monitoring_data.db'
BACKUP_INTERVAL = 3600  # Backup a cada hora

# Interface Web
WEB_HOST = '0.0.0.0'
WEB_PORT = 5000
DEBUG = False

# Análise
CHANGE_THRESHOLD = 3.0  # % para considerar mudança significativa
ALERT_THRESHOLD = 12.0  # % para alertas
```

### Variáveis de Ambiente

```bash
# Configurações via variáveis de ambiente
export MQTT_BROKER="192.168.1.100"
export MQTT_USERNAME="seu_usuario"
export MQTT_PASSWORD="sua_senha"
export DATABASE_PATH="/var/lib/esp32monitor/data.db"
```

## 🔧 Uso

### Script Principal

```bash
# Execução básica
python mqtt_data_collector.py

# Com configurações específicas
MQTT_BROKER=192.168.1.100 python mqtt_data_collector.py

# Em background
nohup python mqtt_data_collector.py > logs/collector.log 2>&1 &
```

### Interface Web

```bash
# Iniciar servidor web
python web/app.py

# Acessar em: http://localhost:5000
```

### Scripts Utilitários

```bash
# Análise de dados
python scripts/analyze_data.py --start-date 2025-01-01

# Geração de relatórios
python scripts/generate_report.py --format pdf

# Backup do banco
python scripts/backup_database.py

# Limpeza de dados antigos
python scripts/cleanup_old_data.py --days 30
```

## 📊 Banco de Dados

### Esquema Principal

```sql
-- Tabela de dados de monitoramento
CREATE TABLE monitoring_data (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    device_id TEXT NOT NULL,
    location TEXT,
    image_size INTEGER,
    change_percentage REAL,
    is_significant_change BOOLEAN,
    is_alert BOOLEAN,
    battery_level INTEGER,
    signal_strength INTEGER,
    temperature REAL,
    humidity REAL,
    raw_data TEXT
);

-- Tabela de imagens (opcional)
CREATE TABLE images (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    monitoring_id INTEGER,
    image_data BLOB,
    image_path TEXT,
    capture_time DATETIME,
    FOREIGN KEY (monitoring_id) REFERENCES monitoring_data (id)
);

-- Tabela de alertas
CREATE TABLE alerts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    device_id TEXT,
    alert_type TEXT,
    severity TEXT,
    message TEXT,
    is_resolved BOOLEAN DEFAULT FALSE
);
```

### Consultas Úteis

```sql
-- Dados das últimas 24 horas
SELECT * FROM monitoring_data 
WHERE timestamp > datetime('now', '-1 day')
ORDER BY timestamp DESC;

-- Estatísticas de mudanças
SELECT 
    device_id,
    COUNT(*) as total_captures,
    SUM(is_significant_change) as significant_changes,
    AVG(change_percentage) as avg_change
FROM monitoring_data 
GROUP BY device_id;

-- Alertas não resolvidos
SELECT * FROM alerts 
WHERE is_resolved = FALSE 
ORDER BY timestamp DESC;
```

## 📡 Protocolo MQTT

### Tópicos e Formato

#### Dados de Monitoramento
```
Tópico: monitoring/data
Formato: JSON
```

```json
{
    "timestamp": "2025-01-15T10:30:00Z",
    "device_id": "ESP32CAM_001",
    "location": "Rio_Principal_Sensor01",
    "image_size": 45280,
    "change_percentage": 5.2,
    "is_significant_change": true,
    "is_alert": false,
    "battery_level": 85,
    "signal_strength": -45,
    "temperature": 22.5,
    "humidity": 65.0,
    "memory_usage": 13.6,
    "uptime": 86400
}
```

#### Imagens (Base64)
```
Tópico: monitoring/images
Formato: JSON com imagem em Base64
```

```json
{
    "timestamp": "2025-01-15T10:30:00Z",
    "device_id": "ESP32CAM_001",
    "image_data": "/9j/4AAQSkZJRgABAQEAYABgAAD...",
    "image_format": "jpeg",
    "resolution": "480x320",
    "quality": 5
}
```

#### Status do Sistema
```
Tópico: monitoring/status
```

```json
{
    "timestamp": "2025-01-15T10:30:00Z",
    "device_id": "ESP32CAM_001",
    "status": "online",
    "version": "intelligent",
    "wifi_connected": true,
    "mqtt_connected": true,
    "camera_status": "ok",
    "last_capture": "2025-01-15T10:29:45Z"
}
```

## 🧪 Testes

### Executar Testes

```bash
# Todos os testes
python -m pytest tests/ -v

# Testes específicos
python -m pytest tests/test_mqtt_client.py -v

# Com cobertura
python -m pytest tests/ --cov=. --cov-report=html

# Testes de integração
python -m pytest tests/integration/ -v
```

### Estrutura de Testes

```
tests/
├── __init__.py
├── test_mqtt_client.py         # Testes do cliente MQTT
├── test_database.py            # Testes do banco de dados
├── test_image_processor.py     # Testes de processamento
├── test_web_interface.py       # Testes da interface web
├── integration/                # Testes de integração
│   ├── test_full_pipeline.py
│   └── test_mqtt_to_db.py
└── fixtures/                   # Dados de teste
    ├── sample_data.json
    └── test_images/
```

## 🔍 Monitoramento e Logs

### Configuração de Logs

```python
# logs/logging_config.py
import logging
import logging.handlers

def setup_logging():
    logger = logging.getLogger('esp32_monitor')
    logger.setLevel(logging.INFO)
    
    # Handler para arquivo
    file_handler = logging.handlers.RotatingFileHandler(
        'logs/mqtt_collector.log',
        maxBytes=10*1024*1024,  # 10MB
        backupCount=5
    )
    
    # Formato
    formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)
    
    return logger
```

### Métricas do Sistema

O sistema coleta automaticamente:
- 📊 **Taxa de recepção** de dados MQTT
- 💾 **Uso de disco** do banco de dados
- 🌐 **Status de conectividade** MQTT
- ⚡ **Performance** de processamento
- 🚨 **Alertas** e eventos críticos

## 🚀 Deploy em Produção

### Usando systemd

```bash
# /etc/systemd/system/esp32-monitor.service
[Unit]
Description=ESP32-CAM Flood Monitor Server
After=network.target mosquitto.service

[Service]
Type=simple
User=esp32monitor
WorkingDirectory=/opt/esp32-monitor/server
ExecStart=/opt/esp32-monitor/server/esp32_monitor_env/bin/python mqtt_data_collector.py
Restart=always
RestartSec=10
Environment=PYTHONPATH=/opt/esp32-monitor/server

[Install]
WantedBy=multi-user.target
```

```bash
# Instalar e habilitar
sudo systemctl daemon-reload
sudo systemctl enable esp32-monitor
sudo systemctl start esp32-monitor
```

### Usando Docker

```dockerfile
# Dockerfile
FROM python:3.9-slim

WORKDIR /app
COPY requirements.txt .
RUN pip install -r requirements.txt

COPY . .

EXPOSE 5000
CMD ["python", "mqtt_data_collector.py"]
```

```bash
# Build e execução
docker build -t esp32-monitor-server .
docker run -d -p 5000:5000 -v $(pwd)/data:/app/data esp32-monitor-server
```

## 📈 API REST (Opcional)

### Endpoints Disponíveis

```python
# Dados recentes
GET /api/data/recent?hours=24

# Estatísticas
GET /api/stats/summary

# Alertas
GET /api/alerts/active

# Status do sistema
GET /api/status

# Exportar dados
GET /api/export?start=2025-01-01&end=2025-01-31&format=csv
```

## 🛠️ Desenvolvimento

### Setup de Desenvolvimento

```bash
# Dependências de desenvolvimento
pip install -r requirements-dev.txt

# Pre-commit hooks
pre-commit install

# Executar em modo debug
DEBUG=True python mqtt_data_collector.py
```

### Estrutura de Contribuição

1. **Fork** o repositório
2. **Clone** sua fork
3. **Crie** uma branch para sua feature
4. **Desenvolva** seguindo os padrões
5. **Teste** suas mudanças
6. **Envie** um Pull Request

## 📞 Suporte

- 🐛 **Issues:** [GitHub Issues](https://github.com/seu-usuario/esp32-cam-flood-monitor/issues)
- 📧 **Email:** gabriel.passos@unesp.br
- 📖 **Documentação:** [Docs Completas](../docs/)

## 📄 Licença

Este projeto está licenciado sob a Licença MIT - veja o arquivo [LICENSE](../LICENSE) para detalhes.

---

> 💡 **Dica:** Para melhor performance em produção, considere usar um broker MQTT externo como HiveMQ ou AWS IoT Core. 
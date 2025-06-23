# 🐍 Servidor de Monitoramento Python

Este diretório contém o sistema de monitoramento científico desenvolvido em Python para coleta, análise e geração de relatórios dos dados do ESP32-CAM.

## 📁 **Estrutura do Servidor**

```
server/
├── mqtt_data_collector.py     # Coletor principal de dados MQTT
├── ic_monitor.py.backup       # Backup da versão anterior
├── requirements_ic.txt        # Dependências Python
└── README.md                  # Este arquivo
```

**Bancos de dados são criados automaticamente em `../data/databases/`**

## 🔬 **Sistema Científico de Monitoramento**

### **Coletor Principal** (`mqtt_data_collector.py`)
- **Função**: Coleta dados MQTT e armazena em SQLite
- **Bancos**: Separados por versão (intelligent/simple)
- **Detecção**: Automática da versão baseada nos dados
- **Imagens**: Organizadas em `../data/images/`
- **Estatísticas**: Tempo real com thread dedicada

### **Foco do Servidor**
- **Coleta contínua**: Dados MQTT em tempo real
- **Armazenamento**: Bancos SQLite organizados
- **Detecção automática**: Versão baseada nos dados
- **Estatísticas live**: Monitoramento em tempo real

## 🚀 **Instalação e Uso**

### **Instalação Automática**
```bash
# Da pasta raiz do projeto
./scripts/setup.sh
# Escolha: 3) Configurar servidor Python
```

### **Executar Monitor**
```bash
cd server
python3 mqtt_data_collector.py
```

### **Gerar Relatórios**
```bash
# Relatórios científicos (executar da pasta scripts)
cd scripts
python3 generate_report.py
```

## 📊 **Funcionalidades Científicas**

- ✅ **Dual Database**: Bancos separados por versão em `../data/databases/`
- ✅ **Auto-detecção**: Identifica versão pelos dados recebidos
- ✅ **Imagens organizadas**: Por versão em `../data/images/`
- ✅ **Métricas completas**: Rede, sistema, detecção, performance
- ✅ **Gráficos científicos**: Relatórios gerados em `../data/reports/`

## 📋 **Descrição Detalhada**

O `mqtt_data_collector.py` é o servidor responsável por:
- Conectar ao broker MQTT e receber dados do ESP32-CAM
- Separar dados por versão (INTELLIGENT vs SIMPLE)
- Salvar em bancos de dados SQLite distintos
- Armazenar imagens capturadas
- Gerar estatísticas em tempo real

## 🚀 **Uso**

### Execução básica:
```bash
python3 mqtt_data_collector.py
```

### Com versão forçada:
```bash
# Força todos os dados para versão INTELLIGENT
python3 mqtt_data_collector.py --version intelligent

# Força todos os dados para versão SIMPLE
python3 mqtt_data_collector.py --version simple
```

## 📊 **Estrutura de Dados**

### Bancos de dados:
- `data/databases/monitoring_intelligent.db` - Dados da versão inteligente
- `data/databases/monitoring_simple.db` - Dados da versão simples

### Tabelas:
- **images**: Registro de imagens capturadas
- **alerts**: Alertas de mudanças significativas
- **monitoring_data**: Dados contínuos de monitoramento
- **system_status**: Status do sistema ESP32
- **network_traffic**: Estatísticas de rede
- **performance_metrics**: Métricas de desempenho

### Imagens:
- `data/images/intelligent/` - Imagens da versão inteligente
- `data/images/simple/` - Imagens da versão simples

## 🔧 **Configuração**

### Variáveis principais:
```python
MQTT_BROKER = "localhost"  # IP do broker MQTT
MQTT_PORT = 1883          # Porta MQTT
MQTT_TOPICS = [           # Tópicos inscritos
    "monitoring/data",
    "monitoring/sniffer/stats",
    "esp32cam/status",
    "esp32cam/alert",
    "esp32cam/image"
]
```

## 📈 **Detecção de Versão**

O servidor detecta automaticamente a versão baseado em:
- Palavras-chave nas mensagens
- Tipo de razão de captura
- Padrões de dados

### Indicadores de versão INTELLIGENT:
- `significant_change`
- `reference_established`
- `anomaly_detected`
- Presença de campo `difference`

### Indicadores de versão SIMPLE:
- `periodic`
- `periodic_sample`
- Ausência de comparação

## 🛠️ **Dependências**

```bash
pip install paho-mqtt
# SQLite3 já vem com Python
```

## 📝 **Logs**

Os logs são salvos em:
- Console: Saída padrão com cores
- Arquivo: `logs/monitor.log`

## 🔍 **Monitoramento**

Para ver dados em tempo real:
```bash
tail -f logs/monitor.log
```

Para verificar integridade dos dados:
```bash
python3 scripts/verify_data_integrity.py
```
 
## 📚 **Documentação Relacionada**

- [Data Structure](../data/README.md) - Scientific data organization
- [Scripts de Automação](../scripts/README.md) - Testes automatizados
- [API MQTT](../docs/mqtt_api.md) - Protocolo de comunicação

---

**Desenvolvido por:** Gabriel Passos - UNESP 2025  
**Propósito:** Coleta e análise científica de dados ESP32-CAM 
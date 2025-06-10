# 📸 Sistema de Monitoramento por Comparação de Imagens ESP32-CAM

## ESP32-CAM + Análise de Mudanças Visuais
**Autor:** Gabriel Passos de Oliveira  
**IGCE/UNESP - 2025**

## 🎯 Objetivos

Sistema de monitoramento inteligente que:

- ✅ **Captura fotos** a cada 15 segundos com ESP32-CAM
- ✅ **Compara imagens** para detectar mudanças visuais
- ✅ **Envia alertas** via MQTT quando detecta mudanças significativas
- ✅ **Salva imagens** quando necessário para análise posterior
- ✅ **Gera relatórios** PDF com estatísticas de monitoramento

## 🏗️ Arquitetura

### ESP32-CAM (Firmware)
```
esp32/main/
├── main.c                  # Sistema principal de monitoramento
├── config.h               # Configurações WiFi/MQTT
└── model/
    ├── compare.c/h         # Algoritmo de comparação de imagens
    ├── init_hw.c/h         # Inicialização de hardware
    ├── init_net.c/h        # Inicialização WiFi/MQTT
    └── mqtt_send.c/h       # Envio de dados via MQTT
```

### Servidor Python
```
server/
├── ic_monitor.py           # Monitor principal com recepção MQTT
├── generate_report.py      # Geração de relatórios PDF
└── requirements_ic.txt     # Dependências Python
```

## 🔧 Hardware

- **ESP32-CAM AI-Thinker** (com PSRAM)
- **Alimentação 5V/2A**
- **WiFi 2.4GHz**
- **Sensor de câmera OV2640**

## ⚙️ Configuração

### 1. ESP32-CAM

```bash
# Instalar ESP-IDF v5.3
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf && ./install.sh && . ./export.sh

# Configurar projeto
cd esp32
cp config.h.example config.h
# Editar config.h com suas credenciais WiFi/MQTT

# Compilar e fazer flash
idf.py build
idf.py -p /dev/ttyUSB0 flash
```

### 2. Servidor Python

```bash
cd server

# Instalar dependências
pip install -r requirements_ic.txt

# Executar monitor
python3 ic_monitor.py
```

### 3. Configuração MQTT

No arquivo `esp32/main/config.h`:
```c
#define WIFI_SSID     "Sua_Rede_WiFi"
#define WIFI_PASS     "Sua_Senha"
#define MQTT_BROKER_URI "mqtt://192.168.1.29:1883"
#define MQTT_USERNAME   "usuario"
#define MQTT_PASSWORD   "senha"
```

## 🔄 Funcionamento

### ESP32-CAM:
1. **A cada 15 segundos**: Captura foto QVGA (320x240)
2. **Comparação inteligente**: Analisa diferenças com imagem anterior
3. **Detecção de mudanças**: Threshold de 10% para mudanças, 30% para alertas
4. **Envio MQTT**: Dados de monitoramento + imagens quando necessário
5. **Algoritmo avançado**: 30 pontos de amostragem distribuídos na imagem

### Servidor Python:
1. **Recebe dados**: Via MQTT com reconexão automática
2. **Processa imagens**: Reconstitui chunks e salva JPEGs
3. **Armazena dados**: SQLite com estatísticas de monitoramento
4. **Gera relatórios**: PDFs com análises e gráficos

## 📡 Tópicos MQTT

```
monitoring/data                        # Dados de monitoramento
monitoring/alert                       # Alertas de mudanças
monitoring/image/metadata              # Metadados de imagem
monitoring/image/data/{timestamp}/{offset}  # Chunks da imagem
```

## 🚀 Execução

### Terminal 1 - Servidor
```bash
cd server
python3 ic_monitor.py
```

### Terminal 2 - Monitor ESP32
```bash
cd esp32  
idf.py -p /dev/ttyUSB0 monitor
```

### Terminal 3 - Gerar Relatório
```bash
cd server
python3 generate_report.py
```

## 📊 Recursos

### 🔍 **Algoritmo de Comparação Inteligente:**
- **30 pontos de amostragem** distribuídos estrategicamente
- **Análise de diferenças significativas** (threshold > 40)
- **Combinação de métricas**: tamanho + conteúdo + mudanças
- **Filtro de ruído** para reduzir falsos positivos

### 📈 **Monitoramento em Tempo Real:**
- **Intervalo de 15 segundos** para detecção rápida
- **Thresholds configuráveis**: 10% mudança, 30% alerta
- **Envio inteligente de imagens** apenas quando necessário

### 📊 **Relatórios Automáticos:**
- **Estatísticas detalhadas** de sistema
- **Últimas 20 leituras** com timestamps
- **Últimos 10 alertas** com análise
- **Geração em PDF** profissional

## 📈 Estatísticas Típicas

| Métrica | Valor Esperado |
|---------|-------------|
| **Intervalo de captura** | 15 segundos |
| **Tamanho por foto** | 5-8 KB (QVGA JPEG Q10) |
| **Precision** | 30 pontos de análise |
| **Sensibilidade** | 10% mudança mínima |
| **Consumo RAM ESP32** | ~200 KB heap + 4 MB PSRAM |
| **Chunks por imagem** | 5-8 chunks de 1KB |

## 📂 Estrutura Final

```
├── esp32/                         # Firmware ESP32-CAM (C/C++)
│   ├── main/
│   │   ├── main.c                 # Sistema principal (bem documentado)
│   │   ├── config.h               # Configurações centralizadas
│   │   └── model/
│   │       ├── compare.c          # Algoritmo de 30 pontos (documentado)
│   │       ├── init_hw.c          # Hardware: câmera + PSRAM
│   │       ├── init_net.c         # WiFi + MQTT
│   │       └── mqtt_send.c        # Envio estruturado MQTT
│   └── CMakeLists.txt
├── server/                        # Servidor Python
│   ├── ic_monitor.py              # Monitor MQTT (bem documentado)
│   ├── generate_report.py         # Relatórios PDF (documentado)
│   ├── monitoring_data.db         # Banco SQLite (3 tabelas)
│   └── received_images/           # Imagens reconstituídas
└── docs/
    └── DOCUMENTACAO_TECNICA.md    # Documentação técnica unificada
```

## 🔧 Configurações Avançadas

### Sensibilidade de Detecção:
```c
#define CHANGE_THRESHOLD 0.10f    // 10% para mudança
#define ALERT_THRESHOLD 0.30f     // 30% para alerta
```

### Intervalo de Captura:
```c
pdMS_TO_TICKS(15000)  // 15 segundos
```

## 📄 Licença

MIT License - Gabriel Passos de Oliveira - IGCE/UNESP 2025

---

*Sistema especializado em detecção de mudanças visuais para monitoramento inteligente.*
# 📸 Sistema de Monitoramento de Enchentes ESP32-CAM

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.0+-blue.svg)](https://github.com/espressif/esp-idf)
[![Python](https://img.shields.io/badge/Python-3.8+-green.svg)](https://www.python.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## Sistema de Detecção de Mudanças Visuais para Monitoramento Fluvial
**Projeto de Iniciação Científica - IGCE/UNESP**  
**Autor:** Gabriel Passos de Oliveira  
**Orientador:** Prof. Dr. Caetano Mazzoni Ranieri  
**Período:** 2025

---

## 📋 Resumo

Sistema embarcado de monitoramento contínuo do nível de rios utilizando ESP32-CAM com processamento local de imagens. O projeto implementa um algoritmo de detecção de mudanças baseado em análise comparativa de frames JPEG, otimizado para ambientes com recursos limitados e conectividade intermitente.

### 🎯 Características Principais

- **Captura**: Imagens QVGA (320x240) a cada 15 segundos
- **Análise**: Algoritmo de comparação por tamanho JPEG
- **Economia**: Transmissão apenas quando detecta mudanças (>1%)
- **Alertas**: Notificação automática para mudanças significativas (>8%)
- **Monitoramento**: WiFi sniffer para análise de consumo de banda

## 📁 Estrutura do Projeto

```
ESP32-IC_Project/
├── esp32/              # Firmware ESP32-CAM (C/ESP-IDF)
├── server/             # Monitor Python + SQLite
├── scripts/            # Scripts de automação
└── docs/               # Documentação técnica
    ├── DOCUMENTACAO_TECNICA.md    # Arquitetura e especificações
    ├── ESP32-CAM_README.md        # Manual do hardware
    ├── INSTALACAO.md              # Guia de instalação
    └── API_MQTT.md                # Protocolo de comunicação
```

## 🚀 Início Rápido

### Pré-requisitos

- ESP-IDF v5.0+ ([Guia de Instalação](docs/INSTALACAO.md#esp-idf))
- Python 3.8+ com pip
- Hardware: ESP32-CAM AI-Thinker + FTDI

### Instalação Básica

```bash
# 1. Clone o repositório
git clone https://github.com/usuario/ESP32-IC_Project.git
cd ESP32-IC_Project

# 2. Use o script de configuração
cd scripts
./setup.sh

# Opção 1: Verificar dependências
# Opção 2: Configurar projeto
# Opção 5: Compilar firmware
# Opção 6: Flash ESP32-CAM
```

Para instruções detalhadas, consulte o [Guia de Instalação](docs/INSTALACAO.md).

## 📊 Métricas do Sistema

| Métrica | Valor | Descrição |
|---------|-------|-----------|
| Taxa de Captura | 4 fps | Máximo em QVGA |
| Consumo Médio | 240mA @ 5V | Com WiFi ativo |
| Precisão | 92% | Em condições controladas |
| Redução de Dados | 95% | Vs. envio contínuo |

## 📡 Arquitetura

```
ESP32-CAM ──MQTT──> Broker ──MQTT──> Python Server
    │                                      │
    └── Análise Local                      └── SQLite DB
```

Detalhes completos em [Documentação Técnica](docs/DOCUMENTACAO_TECNICA.md).

## 🔧 Configuração

Edite `esp32/main/config.h`:

```c
#define WIFI_SSID        "SUA_REDE_2.4GHZ"
#define WIFI_PASS        "SUA_SENHA"
#define MQTT_BROKER_URI  "mqtt://IP_BROKER:1883"
```

Mais opções em [Documentação Técnica](docs/DOCUMENTACAO_TECNICA.md#configuração-e-deploy).

## 📈 Monitoramento

```bash
# Iniciar servidor Python
cd server
source venv/bin/activate
python3 ic_monitor.py

# Gerar relatório PDF
cd scripts
python3 generate_report.py
```

## 🐛 Troubleshooting

Problemas comuns e soluções em [ESP32-CAM Manual](docs/ESP32-CAM_README.md#troubleshooting).

## 📚 Documentação

- [Documentação Técnica](docs/DOCUMENTACAO_TECNICA.md) - Arquitetura e especificações
- [Manual ESP32-CAM](docs/ESP32-CAM_README.md) - Hardware e pinout
- [Guia de Instalação](docs/INSTALACAO.md) - Setup detalhado
- [API MQTT](docs/API_MQTT.md) - Protocolo de comunicação

## 📄 Licença

Este projeto está licenciado sob a MIT License - veja [LICENSE](LICENSE).

## 👥 Contato

**Gabriel Passos de Oliveira**  
📧 gabriel.passos@unesp.br  
🏛️ Instituto de Geociências e Ciências Exatas - IGCE/UNESP  
📍 Rio Claro, SP - Brasil

---

*Projeto desenvolvido como parte do Programa de Iniciação Científica (sem bolsa) 2025*
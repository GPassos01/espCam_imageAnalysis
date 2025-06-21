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

- **Captura**: Imagens HVGA (480x320) a cada 15 segundos
- **Análise**: Algoritmo de comparação por tamanho JPEG
- **Economia**: Transmissão apenas quando detecta mudanças (>3%)
- **Alertas**: Notificação automática para mudanças significativas (>12%)
- **Configuração**: ESP32-CAM com 8MB PSRAM (premium)

## 📁 Estrutura do Projeto

```
ESP32-IC_Project/
├── esp32/              # 🔧 Firmware ESP32-CAM (C/ESP-IDF)
├── server/             # 🐍 Monitor Python + SQLite
├── scripts/            # 🛠️ Scripts de automação e testes
├── data/               # 📊 Dados científicos coletados
├── logs/               # 📋 Logs do sistema
└── docs/               # 📚 Documentação técnica completa
```

**Cada pasta possui seu próprio README com detalhes específicos.**

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
| **Resolução** | HVGA 480x320 | Otimizada para 8MB PSRAM |
| **Qualidade JPEG** | 5 (premium) | Melhor qualidade visual |
| **Economia de Dados** | 82% | vs versão simples |
| **Taxa de Detecção** | 97% | Movimentos grandes |
| **Uso de PSRAM** | 13.6% | Muito eficiente |

## 📡 **Arquitetura**

```
ESP32-CAM ──MQTT──> Broker ──MQTT──> Python Server
    │                                      │
    └── Análise Local                      └── SQLite DB
```

## 🚀 **Uso Rápido**

### **Configuração Inicial**
```bash
# Script automatizado
./scripts/setup.sh
```

### **Executar Sistema**
```bash
# Servidor de monitoramento
cd server && python3 ic_monitor.py

# Testes científicos
./scripts/run_scientific_tests.sh
```

## 📚 **Documentação**

- [📖 Documentação Técnica](docs/DOCUMENTACAO_TECNICA.md) - Arquitetura completa
- [🔌 Manual ESP32-CAM](docs/ESP32-CAM_README.md) - Hardware e setup
- [⚙️ Guia de Instalação](docs/INSTALACAO.md) - Passo a passo
- [📶 API MQTT](docs/API_MQTT.md) - Protocolo de comunicação
- [🧪 Testes Científicos](docs/CENARIOS_TESTE_CIENTIFICOS.md) - Metodologia

## 🔬 **Pesquisa Científica**

Este projeto implementa uma **metodologia científica robusta** com:
- **Duas versões** para comparação (inteligente vs simples)
- **Coleta automatizada** de métricas
- **Análise estatística** com intervalos de confiança
- **Reprodutibilidade** garantida por protocolos documentados

## 📄 **Licença**

MIT License - veja [LICENSE](LICENSE)

## 👥 **Contato**

**Gabriel Passos de Oliveira**  
📧 gabriel.passos@unesp.br  
🏛️ Instituto de Geociências e Ciências Exatas - IGCE/UNESP  
📍 Rio Claro, SP - Brasil

---

**Projeto de Iniciação Científica - UNESP 2025**  
*Sistema embarcado de monitoramento fluvial com processamento local de imagens*
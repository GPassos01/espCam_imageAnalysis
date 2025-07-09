# ESP32-CAM Flood Monitor

<div align="center">

[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](https://choosealicense.com/licenses/mit/)
[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.0+-blue.svg)](https://docs.espressif.com/projects/esp-idf/en/stable/)
[![Python](https://img.shields.io/badge/Python-3.9+-blue.svg)](https://www.python.org/)
[![MQTT](https://img.shields.io/badge/MQTT-5.0-orange.svg)](https://mqtt.org/)
[![Contributions Welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](CONTRIBUTING.md)

*Sistema científico de monitoramento fluvial inteligente usando ESP32-CAM para detecção automática de mudanças visuais em rios*

[Início Rápido](#-início-rápido) • [Documentação](#-documentação) • [Contribuir](#-como-contribuir) • [Suporte](#-suporte)

</div>

---

## Índice

- [Sobre o Projeto](#-sobre-o-projeto)
- [Funcionalidades](#-funcionalidades)
- [Estrutura do Projeto](#-estrutura-do-projeto)
- [Tecnologias Utilizadas](#-tecnologias-utilizadas)
- [Início Rápido](#-início-rápido)
- [Instalação](#-instalação)
- [Uso](#-uso)
- [Arquitetura](#-arquitetura)
- [Documentação](#-documentação)
- [Como Contribuir](#-como-contribuir)
- [Roadmap](#-roadmap)
- [Licença](#-licença)
- [Contato](#-contato)
- [Agradecimentos](#-agradecimentos)

## Sobre o Projeto

O **ESP32-CAM Flood Monitor** é um sistema de monitoramento científico desenvolvido para detectar mudanças visuais em ambientes fluviais usando análise inteligente de imagens. O projeto implementa duas versões para comparação científica:

### Versão Inteligente
- **Análise local avançada** com algoritmo RGB565 otimizado
- **4 tipos de referência**: diurna, noturna, tempo claro e tempo ruim
- **Detecção inteligente** com blocos 32x32 pixels
- **Transmissão seletiva** baseada em mudanças significativas

### Versão Simples (Baseline)
- **Captura contínua** a cada 15 segundos
- **Transmissão total** de todas as imagens
- **Dados básicos** de monitoramento

### Aplicações
- **Monitoramento de enchentes** em tempo real
- **Análise de mudanças ambientais** em rios
- **Pesquisa científica** em hidrologia
- **Sistemas de alerta** para comunidades ribeirinhas

## Funcionalidades

### Análise Avançada de Imagens
- [x] Algoritmo RGB565 otimizado para análise em tempo real
- [x] Sistema de referências múltiplas (dia/noite/clima)
- [x] Detecção de mudanças por blocos 32x32 pixels
- [x] Filtros anti-ruído e correção automática

### Conectividade e Transmissão
- [x] WiFi com reconexão automática
- [x] MQTT 5.0 para transmissão de dados
- [x] Protocolo otimizado para baixo consumo
- [x] Buffer circular para dados históricos

### Recursos de Hardware
- [x] Suporte completo para 8MB PSRAM
- [x] Resolução HVGA (480x320) otimizada
- [x] Sistema anti-esverdeado inteligente
- [x] Monitoramento de recursos (CPU, memória, temperatura)

### Monitoramento e Análise
- [x] Dashboard web em tempo real
- [x] Banco de dados SQLite para armazenamento
- [x] Logs detalhados de eventos
- [x] Métricas de desempenho científico

## Estrutura do Projeto

Esta estrutura segue as [melhores práticas para repositórios GitHub](https://medium.com/code-factory-berlin/github-repository-structure-best-practices-248e6effc405):

```
esp32-cam-flood-monitor/
├── src/                              # Código fonte principal
│   ├── firmware/                     # Firmware ESP32-CAM (C/ESP-IDF)
│   │   ├── main/                     # Código principal
│   │   ├── components/               # Componentes customizados
│   │   └── sdkconfig                 # Configurações ESP-IDF
│   └── server/                       # Servidor Python
│       ├── mqtt_data_collector.py    # Coletor principal MQTT
│       ├── database/                 # Módulos de banco de dados
│       ├── web/                      # Interface web
│       └── analysis/                 # Análise de dados
├── tests/                            # Testes automatizados
│   ├── firmware/                     # Testes do firmware
│   ├── server/                       # Testes do servidor
│   └── integration/                  # Testes de integração
├── examples/                         # Exemplos de uso
│   ├── basic_setup/                  # Setup básico para iniciantes
│   ├── advanced_config/              # Configuração avançada
│   └── scientific_analysis/          # Análise científica
├── tools/                            # Ferramentas de desenvolvimento
│   ├── build/                        # Scripts de build
│   ├── development/                  # Ferramentas de dev
│   ├── deployment/                   # Scripts de deploy
│   └── analysis/                     # Análise científica
├── config/                           #  Arquivos de configuração
│   ├── mqtt/                         # Configurações MQTT
│   ├── wifi/                         # Configurações WiFi
│   └── templates/                    # Templates de config
├── assets/                           # Recursos estáticos
│   ├── images/                       # Imagens da documentação
│   ├── diagrams/                     # Diagramas de arquitetura
│   └── videos/                       # Vídeos demonstrativos
├── build/                            # Scripts de build/CI
│   ├── firmware/                     # Build do firmware
│   └── docker/                       # Containers Docker
├── docs/                             # Documentação completa
│   ├── installation.md               # Guia de instalação
│   ├── configuration.md              #  Configuração avançada
│   ├── api.md                        # Referência da API
│   └── faq.md                        # Perguntas frequentes
├── .github/                          # GitHub específico
│   ├── workflows/                    # GitHub Actions
│   ├── ISSUE_TEMPLATE/               # Templates de issues
│   └── pull_request_template.md      # Template de PR
├── data/                             # Dados científicos
├── logs/                             # Logs do sistema
├── README.md                         # Este arquivo
├── LICENSE                           # Licença MIT
├── CONTRIBUTING.md                   # Guia de contribuição
├── CODE_OF_CONDUCT.md                # Código de conduta
├── CHANGELOG.md                      # Histórico de mudanças
├── SECURITY.md                       # Políticas de segurança
└── SUPPORT.md                        # Guia de suporte
```

## Tecnologias Utilizadas

### Firmware (ESP32-CAM)
- **ESP-IDF v5.0+** - Framework oficial Espressif
- **FreeRTOS** - Sistema operacional em tempo real
- **MQTT** - Protocolo de comunicação IoT
- **JPEG** - Compressão de imagens otimizada

### Servidor e Análise
- **Python 3.9+** - Linguagem principal do servidor
- **MQTT Client** - Cliente MQTT para recepção de dados
- **SQLite** - Banco de dados local
- **Matplotlib/Pandas** - Análise e visualização

### Ferramentas de Desenvolvimento
- **PlatformIO** / **ESP-IDF** - Desenvolvimento do firmware
- **Git** - Controle de versão
- **GitHub Actions** - CI/CD automático

## Início Rápido

### Pré-requisitos

```bash
# Hardware necessário
- ESP32-CAM (recomendado: AI-Thinker)
- Cartão MicroSD (opcional)
- Fonte de alimentação 5V/3A
- Cabo FTDI para programação

# Software necessário
- ESP-IDF v5.0+ ou PlatformIO
- Python 3.9+
- Git
```

### Instalação Rápida

```bash
# 1. Clone o repositório
git clone https://github.com/GPassos01/espCam_imageAnalysis.git
cd esp32-cam-flood-monitor

# 2. Configure o ESP-IDF
. $IDF_PATH/export.sh

# 3. Configure o projeto
cd src/firmware
cp config/templates/config.example.h main/config.h
# Edite config.h com suas configurações WiFi/MQTT

# 4. Compile e faça upload
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor

# 5. Configure o servidor (terminal separado)
cd ../server
pip install -r requirements.txt
python mqtt_data_collector.py
```

## Status dos Testes

### **Funcionalidades Testadas Manualmente**
- **Firmware ESP32-CAM**: Ambas as versões (INTELLIGENT/SIMPLE) funcionando
- **Servidor Python**: Monitor científico coletando dados via MQTT
- **Análise de Imagens**: Algoritmo RGB565 detectando mudanças corretamente
- **Transmissão MQTT**: Comunicação estável entre ESP32-CAM e servidor
- **Banco de Dados**: SQLite armazenando imagens e estatísticas
- **Estrutura de Projeto**: Reestruturação profissional concluída

### **Ferramentas em Beta (Não Testadas)**
> **ATENÇÃO**: As ferramentas de desenvolvimento e scripts automatizados ainda não foram completamente testados após a reestruturação do projeto.

- **Tools de Build**: `tools/build/` - Scripts de compilação automatizada
- **Tools de Development**: `tools/development/` - Ferramentas de desenvolvimento
- **Tools de Deployment**: `tools/deployment/` - Scripts de deploy
- **Tools de Analysis**: `tools/analysis/` - Análise científica automatizada

### **Como Testar o Projeto**

**Para uso básico (recomendado):**
```bash
# 1. Firmware: Compilação manual via ESP-IDF
cd src/firmware
idf.py build flash monitor

# 2. Servidor: Execução manual
cd src/server  
source venv/bin/activate
python3 mqtt_data_collector.py
```

**Para testar diferentes versões:**
```bash
# Testar versão SIMPLE (baseline)
python3 mqtt_data_collector.py --force-version simple

# Testar versão INTELLIGENT (economia de dados)  
python3 mqtt_data_collector.py --force-version intelligent
```

## Documentação

### Documentação Completa
- **[Guia de Instalação](docs/installation.md)** - Instalação detalhada passo a passo
- **[Configuração](docs/configuration.md)** - Configuração avançada do sistema
- **[API Reference](docs/api.md)** - Referência completa da API
- **[Análise de Imagens](docs/image-analysis.md)** - Detalhes do algoritmo de análise
- **[Troubleshooting](docs/troubleshooting.md)** - Solução de problemas comuns

### Guias Técnicos
- **[Protocolo MQTT](docs/mqtt-protocol.md)** - Especificação do protocolo
- **[Hardware Setup](docs/hardware.md)** - Configuração de hardware
- **[Performance Tuning](docs/performance.md)** - Otimização de performance

## Arquitetura

```
┌─────────────────┐    MQTT     ┌──────────────────┐    SQLite    ┌─────────────────┐
│   ESP32-CAM     │ ──────────► │   MQTT Broker    │ ───────────► │   Data Storage  │
│                 │             │   (Mosquitto)    │              │   (SQLite DB)   │
│ • Captura       │             └──────────────────┘              └─────────────────┘
│ • Análise       │                      │                                  │
│ • Transmissão   │                      ▼                                  ▼
└─────────────────┘             ┌──────────────────┐              ┌─────────────────┐
                                │  Data Collector  │              │   Web Dashboard │
                                │   (Python)       │              │   (Flask/HTML)  │
                                └──────────────────┘              └─────────────────┘
```

## Como Contribuir

Adoramos contribuições! Por favor, leia nosso [Guia de Contribuição](CONTRIBUTING.md) para detalhes sobre nosso código de conduta e o processo para enviar pull requests.

### Reportando Bugs
Use os [templates de issue](.github/ISSUE_TEMPLATE/) para reportar bugs ou solicitar funcionalidades.

### Sugestões de Melhorias
1. Faça fork do projeto
2. Crie sua feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a branch (`git push origin feature/AmazingFeature`)
5. Abra um Pull Request

## Roadmap

### v2.0.0 (Próxima Release)
- [ ] **Análise ML/IA** - Implementação de modelos de machine learning
- [ ] **Multi-sensor** - Suporte para sensores adicionais (temperatura, umidade)
- [ ] **Cloud Integration** - Integração com AWS IoT/Google Cloud
- [ ] **Mobile App** - Aplicativo móvel para monitoramento

### v1.1.0 (Em Desenvolvimento)
- [ ] **Dashboard Web** - Interface web responsiva
- [ ] **Alertas configuráveis** - Sistema de notificações personalizáveis
- [ ] **API REST** - API completa para integração externa
- [ ] **Backup automático** - Sistema de backup de dados

Veja a [lista completa de issues](https://github.com/GPassos01/espCam_imageAnalysis/issues) para propostas de funcionalidades e bugs conhecidos.

## Licença

Distribuído sob a Licença MIT. Veja `LICENSE` para mais informações.

## Contato

**Gabriel Passos** - gabriel.passos@unesp.br

**Link do Projeto:** [https://github.com/GPassos01/espCam_imageAnalysis](https://github.com/GPassos01/espCam_imageAnalysis)

## Agradecimentos

* [ESP-IDF](https://docs.espressif.com/projects/esp-idf/) - Framework oficial Espressif
* [MQTT.org](https://mqtt.org/) - Protocolo de comunicação IoT
* [Choose an Open Source License](https://choosealicense.com) - Guia de licenças
* [Img Shields](https://shields.io) - Badges para README
* [Best-README-Template](https://github.com/othneildrew/Best-README-Template) - Template base

---

<div align="center">
  
**Se este projeto foi útil, considere dar uma estrela!**

[![Star History Chart](https://api.star-history.com/svg?repos=GPassos01/espCam_imageAnalysis&type=Date)](https://star-history.com/#GPassos01/espCam_imageAnalysis&Date)

</div>

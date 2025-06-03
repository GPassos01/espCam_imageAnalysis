# 🌊 Sistema de Monitoramento de Enchentes com ESP32-CAM

## Projeto de Iniciação Científica - IGCE/UNESP
**Autor:** Gabriel Passos de Oliveira  
**Orientador:** Prof. Dr. Caetano Mazzoni Ranieri  
**Ano:** 2025

## 📋 Descrição

Sistema inteligente de monitoramento de enchentes utilizando **ESP32-CAM** com **câmera OV2640** para análise de imagens em tempo real, detecção de mudanças significativas e comunicação via MQTT.

## 🎥 ESP32-CAM Especificações

- **Microcontrolador:** ESP32-S (Dual Core 240MHz)
- **Câmera:** OV2640 (2 Megapixels)
- **Memória:** 4MB Flash + 8MB PSRAM
- **Conectividade:** WiFi 802.11 b/g/n
- **LED Flash:** GPIO4 integrado
- **Alimentação:** 5V via USB ou 3.3V
- **Resolução configurada:** 320x240 JPEG para otimização

## 🏗️ Estrutura do Projeto

```
wifi_sniffer/
├── 📁 docs/                          # Documentação do projeto
│   └── Projeto_IC_Gabriel_Passos.pdf
├── 📁 esp32/                         # Firmware ESP32-CAM
│   ├── 📁 main/                      # Código principal da câmera
│   │   └── main.c                    # Sistema de captura e análise
│   ├── partitions.csv                # Tabela de partições otimizada
│   ├── sdkconfig.defaults            # Configurações ESP32-CAM
│   └── CMakeLists.txt                # Build configuration
├── 📁 imagens/                       # Imagens de teste (deprecated)
├── 📁 scripts/                       # Scripts utilitários
│   ├── setup.sh                     # Script de configuração ESP32-CAM
│   └── teste_imagens.py              # Algoritmos de comparação
├── 📁 server/                        # Sistema de monitoramento
│   ├── monitor_mqtt.py               # Monitor MQTT avançado
│   ├── validar_dados.py              # Validação de dados
│   ├── requirements.txt              # Dependências Python
│   └── README_monitor.md             # Documentação do monitor
└── README.md                         # Este arquivo
```

## 🚀 Funcionalidades

### ESP32-CAM (Firmware)
- ✅ **Captura real de imagens** com câmera OV2640
- ✅ **Processamento JPEG nativo** (320x240 pixels)
- ✅ **Análise comparativa entre imagens consecutivas** 
- ✅ **Envio de pares de imagens** quando diferença > 15%
- ✅ **Flash LED automático** para melhor iluminação
- ✅ **Comunicação MQTT** com transmissão em chunks
- ✅ **Sistema de alertas** para mudanças significativas (>50%)
- ✅ **Identificação única de pares** para correlação
- ✅ **Monitoramento PSRAM** e uso de memória

### Sistema de Monitoramento (Python)
- ✅ **Monitor MQTT simplificado** para ESP32-CAM
- ✅ **Recepção e reconstituição de pares de imagens**
- ✅ **Banco de dados SQLite** para armazenamento histórico
- ✅ **Extrator de imagens** com visualização
- ✅ **Validação e análise** de dados da câmera
- ✅ **Sistema de alertas** baseado em diferenças

## 🔄 Fluxo de Análise de Imagens

### 1. **Captura e Comparação**
```
ESP32-CAM → Captura Imagem A → Armazena como "anterior"
ESP32-CAM → Captura Imagem B → Compara com A
Se diferença > 15% → Enviar par (A + B) via MQTT
```

### 2. **Transmissão MQTT**
```
Tópicos gerados automaticamente:
• enchentes/imagem/dados/anterior/{pair_id}/{offset}/{total_size}
• enchentes/imagem/dados/atual/{pair_id}/{offset}/{total_size}
• enchentes/sensores (metadados do par)
• enchentes/alertas (se diferença > 50%)
```

### 3. **Reconstituição e Visualização**
```
Monitor Python → Recebe chunks → Reconstitui imagens
Banco SQLite → Armazena chunks organizados por pair_id
Script extrator → Reconstitui JPEGs completos para visualização
```

## 🛠️ Configuração e Instalação

### 1. Pré-requisitos Hardware

```
ESP32-CAM AI-Thinker:
- ESP32-S com câmera OV2640
- Programador FTDI USB-Serial (3.3V)
- Jumpers para modo de programação
- Fonte de alimentação 5V/2A
```

### 2. Pré-requisitos Software

```bash
# ESP-IDF (versão 5.0+)
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf && ./install.sh && . ./export.sh

# Componente ESP32-Camera
cd $IDF_PATH/components
git clone https://github.com/espressif/esp32-camera.git

# Python 3.10+
sudo apt update
sudo apt install python3 python3-pip python3-venv
```

### 3. Configuração do Projeto

```bash
# Clone o repositório
git clone <url-do-repositorio>
cd wifi_sniffer

# Execute o script de configuração ESP32-CAM
chmod +x scripts/setup.sh
./scripts/setup.sh
```

### 4. Compilação e Flash ESP32-CAM

```bash
# Use o menu interativo do setup.sh
./scripts/setup.sh

# Ou manualmente:
cd esp32
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

### 5. Conexão ESP32-CAM

```
Modo Programação (Flash):
- GPIO0 -> GND (jumper)
- VCC -> 5V
- GND -> GND
- U0R -> TX (FTDI)
- U0T -> RX (FTDI)

Modo Operação:
- Remover jumper GPIO0-GND
- Reset na ESP32-CAM
```

## 📊 Resultados e Performance

### Análise de Pares de Imagens
- **Intervalo de captura**: 30 segundos entre imagens
- **Threshold de envio**: 15% de diferença
- **Threshold de alerta**: 50% de diferença  
- **Resolução**: 320x240 pixels JPEG
- **Tamanho típico**: 3-8KB por imagem comprimida
- **Identificação única**: Timestamp como pair_id

### Comunicação MQTT
- **Chunks**: 1KB por pacote MQTT
- **Tipos de imagem**: "anterior" e "atual" 
- **Latência**: <200ms por chunk
- **Taxa de sucesso**: >95% das transmissões
- **Tópicos organizados**: Por tipo e pair_id

## 🔧 Configurações

### ESP32-CAM (main/main.c)
```c
#define WIFI_SSID        "Sua_Rede_WiFi"
#define WIFI_PASS        "Sua_Senha"
#define MQTT_BROKER_URI  "mqtt://ip_do_broker:1883"

// Configurações da câmera
#define IMAGE_CAPTURE_INTERVAL  30000   // 30 segundos
#define CHANGE_THRESHOLD        0.15    // 15% de diferença
#define FRAMESIZE_QVGA                  // 320x240 pixels
#define JPEG_QUALITY           10       // Qualidade JPEG (0-63)
```

### Python (server/monitor_mqtt.py)
```python
MQTT_BROKER = "192.168.1.2"
MQTT_PORT = 1883
DATABASE_FILE = "enchentes_data_teste.db"
```

## 📈 Monitoramento e Logs

O sistema ESP32-CAM gera logs detalhados sobre:
- 📸 **Captura de imagens** e qualidade
- 🔍 **Análise de diferenças** entre quadros consecutivos
- 📦 **Compressão JPEG** e otimização de tamanho
- 🌐 **Estatísticas de rede** WiFi e MQTT
- 🚨 **Alertas** de mudanças significativas
- 💾 **Uso de memória** PSRAM e interna
- ⚡ **Performance** de captura e transmissão

## 🧪 Uso do Sistema

### 1. **Iniciar Monitor de Recepção**
```bash
cd server
python3 monitor_mqtt.py
```

### 2. **Visualizar Imagens Capturadas**
```bash
# Ver informações do banco
python3 extract_images.py --info

# Extrair todas as imagens
python3 extract_images.py

# Visualizar imagens extraídas
cd extracted_images
xdg-open *.jpg  # Linux
```

### 3. **Teste de Diferenças**
```bash
# Posicionar ESP32-CAM apontando para cena
# Aguardar primeira captura (30s)
# Fazer mudança significativa na cena
# Observar logs do monitor para par de imagens enviado
```

## 🔄 Próximos Passos

- [ ] **Otimização de energia** para operação com bateria
- [ ] **Implementação de IA/ML** para classificação automática de enchentes
- [ ] **Interface web** para configuração remota da ESP32-CAM
- [ ] **Sistema de notificações** push/email para alertas
- [ ] **Múltiplas câmeras** em rede mesh
- [ ] **Análise de vídeo** em tempo real
- [ ] **Integração com sensores** de nível d'água
- [ ] **Armazenamento na nuvem** das imagens críticas

## 📄 Licença

Este projeto está licenciado sob a MIT License - veja o arquivo [LICENSE](LICENSE) para detalhes.

## 👤 Autor

**Gabriel Passos de Oliveira**  
Projeto de Iniciação Científica  
IGCE/UNESP - 2025  
Email: gabriel.passos@unesp.br  
Orientador: Prof. Dr. Caetano Mazzoni Ranieri  

---

*Sistema desenvolvido para monitoramento inteligente de enchentes utilizando ESP32-CAM com análise de imagens em tempo real e comunicação IoT.*
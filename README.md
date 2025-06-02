# 🌊 Sistema de Monitoramento de Enchentes com ESP32

## Projeto de Iniciação Científica - IGCE/UNESP
**Autor:** Gabriel Passos de Oliveira  
**Orientador:** Prof. Dr. Caetano Mazzoni Ranieri  
**Ano:** 2025

## 📋 Descrição

Sistema inteligente de monitoramento de enchentes utilizando ESP32 com análise de imagens em tempo real, detecção de mudanças significativas e comunicação via MQTT.

## 🏗️ Estrutura do Projeto

```
wifi_sniffer/
├── 📁 docs/                          # Documentação do projeto
│   └── Projeto_IC_Gabriel_Passos.pdf
├── 📁 esp32/                         # Firmware ESP32
│   ├── 📁 main/                      # Código principal
│   ├── 📁 spiffs_image/              # Imagens para SPIFFS
│   ├── partitions.csv                # Tabela de partições
│   ├── sdkconfig.defaults            # Configurações padrão
│   └── CMakeLists.txt
├── 📁 imagens/                       # Imagens de teste
│   ├── img1_gray.jpg                 # Imagem 1 (tons de cinza)
│   ├── img2_gray.jpg                 # Imagem 2 (tons de cinza)
│   ├── img1_320x240.jpg              # Versão redimensionada
│   ├── img2_320x240.jpg              # Versão redimensionada
│   └── diferenca.jpg                 # Visualização das diferenças
├── 📁 scripts/                       # Scripts utilitários
│   ├── copy_images_to_spiffs.py      # Gera imagem SPIFFS
│   ├── teste_imagens.py              # Testa algoritmos de comparação
│   └── setup.sh                     # Script de configuração
├── 📁 server/                        # Sistema de monitoramento
│   ├── monitor_mqtt.py               # Monitor MQTT principal
│   ├── validar_dados.py              # Validação de dados
│   ├── enchentes_data_teste.db       # Banco de dados
│   ├── spiffs_image.bin              # Imagem SPIFFS gerada
│   └── requirements.txt              # Dependências Python
└── README.md                         # Este arquivo
```

## 🚀 Funcionalidades

### ESP32 (Firmware)
- ✅ **Análise de imagens em tons de cinza** (320x240 pixels)
- ✅ **Detecção de mudanças** com algoritmo pixel-a-pixel
- ✅ **Compressão inteligente** baseada na complexidade da imagem
- ✅ **Comunicação MQTT** com transmissão em chunks
- ✅ **Sistema de alertas** para mudanças significativas (>50%)
- ✅ **Armazenamento SPIFFS** para imagens de referência
- ✅ **Monitoramento de rede** com estatísticas em tempo real

### Sistema de Monitoramento (Python)
- ✅ **Monitor MQTT** com logging detalhado
- ✅ **Banco de dados SQLite** para armazenamento
- ✅ **Validação de dados** e análises estatísticas
- ✅ **Interface de monitoramento** em tempo real

## 🛠️ Configuração e Instalação

### 1. Pré-requisitos

```bash
# ESP-IDF (versão 5.3+)
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf && ./install.sh

# Python 3.10+
sudo apt update
sudo apt install python3 python3-pip python3-venv
```

### 2. Configuração do Ambiente

```bash
# Clone o repositório
git clone <url-do-repositorio>
cd wifi_sniffer

# Execute o script de configuração
chmod +x scripts/setup.sh
./scripts/setup.sh
```

### 3. Compilação e Flash do ESP32

```bash
# Carregue o ambiente ESP-IDF
. $HOME/esp/esp-idf/export.sh

# Compile e grave o firmware
cd esp32
idf.py build flash

# Gere e grave a imagem SPIFFS
cd ../scripts
python3 copy_images_to_spiffs.py
```

### 4. Execução do Monitor

```bash
cd server
python3 monitor_mqtt.py
```

## 📊 Resultados e Performance

### Processamento de Imagens
- **Tamanho das imagens**: 320x240 pixels (tons de cinza)
- **Compressão**: 65-85% de redução de tamanho
- **Detecção de diferenças**: ~33% entre imagens de teste
- **Threshold de alerta**: 12% (configurável)

### Comunicação de Rede
- **Protocolo**: MQTT over WiFi
- **Transmissão**: Chunks de 1KB
- **Latência**: < 100ms por chunk
- **Eficiência**: 0% de imagens descartadas (todas são significativas)

### Uso de Memória
- **ESP32**: ~80KB de RAM livre durante operação
- **SPIFFS**: 1MB partição, ~70KB usado
- **Flash**: 4MB total, 56% livre após firmware

## 🔧 Configurações

### ESP32 (main/main.c)
```c
#define WIFI_SSID        "Sua_Rede_WiFi"
#define WIFI_PASS        "Sua_Senha"
#define MQTT_BROKER_URI  "mqtt://ip_do_broker:1883"
#define CHANGE_THRESHOLD 0.12    // 12% de diferença
#define IMAGE_CAPTURE_INTERVAL 15000  // 15 segundos
```

### Python (server/monitor_mqtt.py)
```python
MQTT_BROKER = "192.168.1.2"
MQTT_PORT = 1883
DATABASE_FILE = "enchentes_data_teste.db"
```

## 📈 Monitoramento e Logs

O sistema gera logs detalhados sobre:
- 📸 **Captura de imagens** e processamento
- 🔍 **Análise de diferenças** entre quadros
- 📦 **Compressão** e otimização
- 🌐 **Estatísticas de rede** e conectividade
- 🚨 **Alertas** de mudanças significativas

## 🧪 Testes

```bash
# Teste do algoritmo de comparação de imagens
cd scripts
python3 teste_imagens.py

# Validação dos dados coletados
cd server
python3 validar_dados.py
```

## 🔄 Próximos Passos

- [ ] Integração com câmera real (OV2640)
- [ ] Implementação de IA/ML para classificação de enchentes
- [ ] Interface web para monitoramento remoto
- [ ] Sistema de notificações (email/SMS)
- [ ] Otimização de consumo energético

## 📄 Licença

Este projeto está licenciado sob a MIT License - veja o arquivo [LICENSE](LICENSE) para detalhes.

## 👤 Autor

**Gabriel Passos de Oliveira**  
Projeto de Iniciação Científica  
IGCE/UNESP - 2024  
Email: gabriel.passos@unesp.br  
Orientador: Prof. Dr. Caetano Mazzoni Ranieri  
Ano: 2025

---

*Sistema desenvolvido para monitoramento inteligente de enchentes utilizando tecnologias IoT e processamento de imagens.*
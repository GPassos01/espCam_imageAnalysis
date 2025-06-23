# 🔧 Firmware ESP32-CAM

Este diretório contém o firmware embarcado para o ESP32-CAM, desenvolvido em C usando o framework ESP-IDF.

## 📁 **Estrutura do Firmware**

```
esp32/
├── main/                       # Código fonte principal
│   ├── main.c                 # Versão inteligente (padrão)
│   ├── main_simple.c          # Versão simples (baseline)
│   ├── config.h               # Configurações do sistema
│   ├── model/                 # Módulos funcionais
│   │   ├── init_hw.c/h       # Inicialização de hardware
│   │   ├── init_net.c/h      # Configuração de rede
│   │   ├── compare.c/h       # Algoritmo de comparação
│   │   ├── mqtt_send.c/h     # Cliente MQTT
│   │   ├── wifi_sniffer.c/h  # Monitor de tráfego WiFi
│   │   ├── advanced_analysis.c/h  # Análise avançada (8MB PSRAM)
│   │   └── chip_info.c/h     # Informações do chip
│   └── CMakeLists.txt        # Build system
├── CMakeLists.txt             # Configuração do projeto
├── partitions.csv             # Tabela de partições
├── sdkconfig.defaults         # Configurações padrão ESP-IDF
└── dependencies.lock          # Lock de dependências
```

## 🚀 **Duas Versões Disponíveis**

### 🧠 **Versão Inteligente** (`main.c`)
- **Função**: Análise local com envio seletivo
- **Economia**: 82% redução de dados
- **Recursos**: Detecção de mudanças, alertas, histórico
- **Uso**: Produção com dados limitados

### 📸 **Versão Simples** (`main_simple.c`)
- **Função**: Envio contínuo de todas as imagens
- **Dados**: 100% das capturas transmitidas
- **Recursos**: Baseline para comparação científica
- **Uso**: Testes e validação

## ⚙️ **Configurações Principais**

### **Hardware** (`config.h`)
```c
// Resolução otimizada para 8MB PSRAM
#define CAMERA_FRAME_SIZE FRAMESIZE_HVGA  // 480x320
#define CAMERA_JPEG_QUALITY 5             // Qualidade premium

// Thresholds de detecção
#define CHANGE_THRESHOLD 3                // 3% mudança
#define ALERT_THRESHOLD 12               // 12% alerta
```

### **Rede** (`config.h`)
```c
#define WIFI_SSID        "SUA_REDE_2.4GHZ"
#define WIFI_PASS        "SUA_SENHA"
#define MQTT_BROKER_URI  "mqtt://IP_BROKER:1883"
```

## 🔨 **Compilação e Deploy**

### **Método 1: Script Automatizado**
```bash
# Da pasta raiz do projeto
./scripts/setup.sh
# Escolha: 5) Compilar firmware
# Escolha: 6) Flash ESP32-CAM
```

### **Método 2: Manual**
```bash
cd esp32
idf.py build
idf.py flash monitor
```

### **Alternância de Versões**
```bash
# Da pasta raiz
./scripts/switch_version.sh
# 1) Versão Inteligente
# 2) Versão Simples
```

## 📊 **Especificações Técnicas**

### **Memória (8MB PSRAM física, 4MB utilizável)**
- **Buffer principal**: 210KB (3 imagens HVGA)
- **Referências múltiplas**: 280KB (4 contextos)
- **Análise avançada**: 200KB (algoritmos)
- **Memória livre**: ~3.2MB disponível
- **Utilização**: 13.6% da PSRAM utilizável

### **Performance**
- **Intervalo de captura**: 15 segundos
- **Tempo de processamento**: 60ms (comparação)
- **Latência MQTT**: ~200ms
- **Taxa de detecção**: 97% (movimentos grandes)

### **Recursos Avançados**
- ✅ **Sistema anti-esverdeado**: Detecção e correção automática
- ✅ **Buffer histórico**: 3 imagens para análise temporal
- ✅ **Referências adaptativas**: Contextos por horário/clima
- ✅ **WiFi sniffer**: Monitoramento de tráfego de rede
- ✅ **Análise de tendências**: Detecção de padrões

## 🔧 **Módulos Funcionais**

### **`init_hw.c/h`** - Hardware
- Inicialização da câmera OV2640
- Configuração de PSRAM (8MB)
- Sistema de warm-up inteligente
- Detecção e correção de imagens esverdeadas

### **`init_net.c/h`** - Rede
- Conexão WiFi automática
- Cliente MQTT com reconnect
- Configuração de broker
- Monitoramento de conectividade

### **`compare.c/h`** - Análise
- Algoritmo de comparação por tamanho JPEG
- Detecção de mudanças (3% threshold)
- Sistema de alertas (12% threshold)
- Otimização para HVGA

### **`mqtt_send.c/h`** - Comunicação
- Envio de imagens Base64
- Mensagens de status
- Métricas de sistema
- Protocolo científico

### **`advanced_analysis.c/h`** - Análise Avançada
- Buffer histórico de imagens
- Referências múltiplas por contexto
- Análise temporal e tendências
- Detecção de anomalias

### **`wifi_sniffer.c/h`** - Monitoramento
- Captura de tráfego WiFi
- Análise de consumo de banda
- Estatísticas de rede
- Métricas científicas

## 🐛 **Troubleshooting**

### **Problemas Comuns**

#### **Compilação Falha**
```bash
# Limpar build e recompilar
idf.py clean
idf.py build
```

#### **Camera Probe Failed**
```bash
# Verificar conexões de hardware
# Reiniciar ESP32-CAM
# Verificar alimentação (5V, 2A)
```

#### **WiFi Não Conecta**
```bash
# Verificar SSID/senha em config.h
# Usar rede 2.4GHz (não 5GHz)
# Verificar força do sinal
```

#### **MQTT Timeout**
```bash
# Verificar IP do broker
# Testar conectividade: ping IP_BROKER
# Verificar firewall
```

### **Logs de Debug**
```bash
# Monitor serial com logs detalhados
idf.py monitor

# Filtrar logs específicos
idf.py monitor | grep "INIT_HW"
```

## 📈 **Monitoramento**

### **Métricas Importantes**
- **PSRAM livre**: Deve manter >2MB
- **Heap livre**: Deve manter >100KB
- **Taxa de detecção**: ~97% para movimentos grandes
- **Falsos positivos**: <8% em ambiente controlado

### **Logs de Saúde**
```
I MAIN: 📊 Sistema - Heap: 156KB, PSRAM: 3.2MB livre
I COMPARE: 📈 Detecção - Mudança: 5.2%, Alerta: NÃO
I MQTT: 📡 Envio - Sucesso: 1.2KB em 187ms
```

## 🔒 **Segurança**

### **Configurações Recomendadas**
- Usar credenciais WiFi seguras
- Configurar MQTT com autenticação
- Atualizar firmware regularmente
- Monitorar logs de segurança

### **Dados Sensíveis**
- Credenciais em `config.h` (não commitado)
- Imagens transmitidas via MQTT
- Logs podem conter informações de rede

## 📚 **Documentação Relacionada**

- [Manual ESP32-CAM](../docs/hardware_guide.md) - Hardware e pinout
- [API MQTT](../docs/mqtt_api.md) - Protocolo de comunicação
- [Guia de Instalação](../docs/installation.md) - Setup completo
- [Documentação Técnica](../docs/technical_guide.md) - Arquitetura

---

**Desenvolvido por:** Gabriel Passos - UNESP 2025  
**Framework:** ESP-IDF v5.0+  
**Hardware:** ESP32-CAM AI-Thinker (8MB PSRAM física, 4MB utilizável) 
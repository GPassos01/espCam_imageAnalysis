# 📷 Guia Completo ESP32-CAM - Sistema de Monitoramento de Enchentes

## Projeto de Iniciação Científica - Gabriel Passos - IGCE/UNESP 2025

---

## 🎯 Introdução

Este guia documenta a migração do sistema de monitoramento de enchentes de simulação para **ESP32-CAM real** com câmera **OV2640**. O sistema agora captura imagens reais, processa em tempo real e detecta mudanças significativas para monitoramento de enchentes.

## 🔧 Hardware ESP32-CAM

### Especificações Técnicas
- **Chip Principal:** ESP32-S (Dual Core Xtensa 32-bit 240MHz)
- **Câmera:** OV2640 CMOS 2MP (1600x1200 máximo)
- **Memória Flash:** 4MB
- **PSRAM:** 8MB (crucial para processamento de imagens)
- **WiFi:** 802.11 b/g/n (2.4GHz)
- **Bluetooth:** v4.2 BR/EDR e BLE
- **GPIO Disponíveis:** Limitados devido aos pinos da câmera
- **LED Flash:** GPIO4 (integrado)

### Pinout ESP32-CAM AI-Thinker

```
Pinos da Câmera (NÃO ALTERAR):
┌─────────────────────────────┐
│ OV2640 Camera Configuration │
├─────────────────────────────┤
│ PWDN  │ GPIO32             │
│ RESET │ -1 (not connected) │
│ XCLK  │ GPIO0              │
│ SDA   │ GPIO26             │
│ SCL   │ GPIO27             │
│ D7    │ GPIO35             │
│ D6    │ GPIO34             │
│ D5    │ GPIO39             │
│ D4    │ GPIO36             │
│ D3    │ GPIO21             │
│ D2    │ GPIO19             │
│ D1    │ GPIO18             │
│ D0    │ GPIO5              │
│ VSYNC │ GPIO25             │
│ HREF  │ GPIO23             │
│ PCLK  │ GPIO22             │
└─────────────────────────────┘

Pinos de Programação:
┌─────────────────────────┐
│ VCC   │ 5V (externa)    │
│ GND   │ Ground          │
│ U0R   │ RX (FTDI)       │
│ U0T   │ TX (FTDI)       │
│ GPIO0 │ GND para flash  │
│ RST   │ Reset           │
└─────────────────────────┘

GPIO Disponíveis:
- GPIO1, GPIO3: TX0/RX0 (Serial)
- GPIO12, GPIO13: Disponíveis
- GPIO14, GPIO15: Disponíveis  
- GPIO16: Disponível (PSRAM CS)
- GPIO4: LED Flash (usado no projeto)
```

## ⚡ Configuração Hardware

### 1. Conexão para Programação (Flash)

```
ESP32-CAM          FTDI Programmer
┌─────────┐       ┌─────────────┐
│   VCC   ├───────┤ 5V          │
│   GND   ├───────┤ GND         │
│   U0R   ├───────┤ TX          │
│   U0T   ├───────┤ RX          │
│  GPIO0  ├───┐   │             │
└─────────┘   │   └─────────────┘
              │
             GND (jumper para programação)
```

### 2. Conexão para Operação Normal

```
ESP32-CAM          Fonte
┌─────────┐       ┌─────────────┐
│   VCC   ├───────┤ 5V/2A       │
│   GND   ├───────┤ GND         │
└─────────┘       └─────────────┘

REMOVER jumper GPIO0-GND
```

### 3. Fonte de Alimentação

**⚠️ IMPORTANTE:** A ESP32-CAM requer corrente alta durante operação da câmera:
- **Mínimo:** 500mA durante operação normal
- **Recomendado:** 2A para estabilidade
- **Tensão:** 5V (regulador onboard para 3.3V)

## 🛠️ Configuração Software

### 1. Instalação ESP-IDF 5.0+

```bash
# Clone ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf

# Instalar
./install.sh

# Carregar ambiente (fazer sempre antes de usar)
. ./export.sh
```

### 2. Instalar Componente ESP32-Camera

```bash
# Navegar para componentes do ESP-IDF
cd $IDF_PATH/components

# Clonar componente da câmera
git clone https://github.com/espressif/esp32-camera.git

# Verificar instalação
ls esp32-camera/driver/include/esp_camera.h
```

### 3. Configuração do Projeto

```bash
# No diretório do projeto
cd esp32

# Definir target
idf.py set-target esp32

# Configurar (opcional - já configurado via sdkconfig.defaults)
idf.py menuconfig
```

### 4. Configurações Críticas

**Em `idf.py menuconfig`:**

```
Component config → ESP32-specific:
  ☑ Support for external, SPI-connected RAM
  ☑ SPI RAM config → Initialize SPI RAM when booting the ESP32
  ☑ SPI RAM config → SPI RAM access method → Make RAM allocatable using malloc()

Component config → Camera configuration:
  ☑ OV2640 Support
  ☑ Camera task pinned to core 0

Component config → Wi-Fi:
  - WiFi static RX buffer num: 10
  - WiFi dynamic RX buffer num: 32
  - WiFi dynamic TX buffer num: 32

FreeRTOS:
  ☑ Run FreeRTOS only on first core (unicore)
```

## 🔧 Compilação e Flash

### 1. Compilação

```bash
cd esp32

# Limpar build anterior (se existir)
idf.py clean

# Compilar
idf.py build
```

### 2. Preparação para Flash

```bash
# 1. Conectar FTDI à ESP32-CAM
# 2. Conectar jumper GPIO0-GND
# 3. Conectar fonte 5V
# 4. Reset ESP32-CAM
```

### 3. Flash do Firmware

```bash
# Detectar porta (ex: /dev/ttyUSB0)
ls /dev/ttyUSB*

# Flash
idf.py -p /dev/ttyUSB0 flash

# Monitor (opcional)
idf.py -p /dev/ttyUSB0 monitor
```

### 4. Inicialização

```bash
# 1. Desconectar jumper GPIO0-GND
# 2. Reset ESP32-CAM
# 3. ESP32-CAM deve iniciar normalmente
```

## 📊 Configuração da Câmera

### Configurações Otimizadas (main.c)

```c
static camera_config_t camera_config = {
    .pin_pwdn = 32,
    .pin_reset = -1,
    .pin_xclk = 0,
    .pin_sccb_sda = 26,
    .pin_sccb_scl = 27,
    .pin_d7 = 35,
    .pin_d6 = 34,
    .pin_d5 = 39,
    .pin_d4 = 36,
    .pin_d3 = 21,
    .pin_d2 = 19,
    .pin_d1 = 18,
    .pin_d0 = 5,
    .pin_vsync = 25,
    .pin_href = 23,
    .pin_pclk = 22,
    .xclk_freq_hz = 20000000,       // 20MHz clock
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG, // JPEG compression
    .frame_size = FRAMESIZE_QVGA,   // 320x240
    .jpeg_quality = 10,             // Quality 0-63 (lower=better)
    .fb_count = 2,                  // Double buffering
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY
};
```

### Configurações do Sensor

```c
sensor_t *s = esp_camera_sensor_get();

// Otimizações para detecção de enchentes
s->set_brightness(s, 0);     // Brilho normal
s->set_contrast(s, 2);       // Contraste aumentado
s->set_saturation(s, 0);     // Saturação normal
s->set_whitebal(s, 1);       // White balance automático
s->set_exposure_ctrl(s, 1);  // Exposição automática
s->set_gain_ctrl(s, 1);      // Ganho automático
s->set_hmirror(s, 0);        // Sem espelho horizontal
s->set_vflip(s, 0);          // Sem flip vertical
```

## 💾 Gestão de Memória

### PSRAM (8MB)
```c
// Verificar PSRAM
size_t psram_size = esp_spiram_get_size();
size_t psram_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

// Alocar buffer de imagem em PSRAM
uint8_t *image_buffer = (uint8_t*)heap_caps_malloc(
    MAX_IMAGE_SIZE, MALLOC_CAP_SPIRAM);
```

### RAM Interna (~250KB livre)
```c
// Para estruturas pequenas e críticas
camera_frame_t *frame = malloc(sizeof(camera_frame_t));

// Verificar memória livre
size_t free_heap = esp_get_free_heap_size();
```

## 🔍 Algoritmo de Detecção

### Detecção de Diferenças JPEG

```c
static float calculate_image_difference(camera_frame_t *img1, camera_frame_t *img2) {
    // 1. Verificar diferença de tamanho
    float size_ratio = (float)img1->len / img2->len;
    
    // 2. Amostrar pontos da imagem JPEG
    size_t sample_points = 20;
    uint64_t diff_sum = 0;
    
    for (size_t i = 0; i < sample_points; i++) {
        size_t pos = (i * min_len) / sample_points;
        diff_sum += abs(img1->buf[pos] - img2->buf[pos]);
    }
    
    // 3. Combinar diferenças
    float content_diff = (float)diff_sum / (sample_points * 255);
    float total_diff = (content_diff * 0.7) + (fabs(size_ratio - 1.0f) * 0.3);
    
    return total_diff;
}
```

### Configurações de Alerta

```c
#define CHANGE_THRESHOLD 0.15    // 15% de diferença
#define ALERT_THRESHOLD 0.50     // 50% para alerta de enchente
```

## 📡 Comunicação MQTT

### Dados do Sensor

```json
{
  "timestamp": 1704067200,
  "image_size": 45678,
  "compressed_size": 45678,
  "difference": 0.234,
  "width": 320,
  "height": 240,
  "format": 4,
  "location": "rio_monitoring_esp32cam",
  "modo": "camera_real"
}
```

### Transmissão de Imagem

```
Topic: enchentes/imagem/dados/0/45678
Payload: [chunk 0 de 1024 bytes]

Topic: enchentes/imagem/dados/1024/45678  
Payload: [chunk 1 de 1024 bytes]

...
```

### Alertas

```json
{
  "alert": "significant_change",
  "difference": 0.567,
  "timestamp": 1704067200,
  "image_size": 45678,
  "location": "rio_monitoring_esp32cam",
  "modo": "camera_real"
}
```

## 🧪 Testes e Depuração

### 1. Teste da Câmera

```bash
# Via setup.sh
./scripts/setup.sh
# Opção 13: Testar câmera ESP32-CAM

# Manual
cd esp32
idf.py -p /dev/ttyUSB0 flash monitor
```

### 2. Logs Importantes

```
✅ Inicialização OK:
🎥 Inicializando câmera ESP32-CAM...
✅ Câmera inicializada com sucesso!
📷 Configuração: 320x240 JPEG, qualidade=10

📸 Captura OK:
📸 Imagem capturada: 45678 bytes (320x240), formato=4

🔍 Análise OK:
🔍 Análise: tamanho_ratio=1.05, diff_conteudo=0.234, diff_total=0.267
```

### 3. Problemas Comuns

**Falha na inicialização da câmera:**
```
❌ Falha ao inicializar câmera: ESP_ERR_NOT_FOUND
```
- Verificar conexões dos pinos da câmera
- Verificar fonte de alimentação (min 500mA)
- Verificar componente esp32-camera instalado

**Falha na captura:**
```
❌ Falha na captura da câmera
```
- Verificar PSRAM habilitado
- Verificar qualidade JPEG não muito alta
- Verificar iluminação adequada

**Problemas de memória:**
```
❌ Falha ao alocar memória para buffer de imagem
```
- Verificar PSRAM funcionando
- Reduzir qualidade JPEG
- Verificar vazamentos de memória

## 📈 Performance e Otimização

### Benchmarks Típicos
- **Inicialização:** ~3-5 segundos
- **Captura:** ~200-500ms por imagem
- **Processamento:** ~50-100ms por comparação
- **Transmissão:** ~2-8 segundos (dependendo do tamanho)
- **Intervalo:** 30 segundos entre capturas

### Otimizações Implementadas
1. **JPEG nativo** - reduz processamento
2. **Buffer duplo** - evita perda de frames  
3. **PSRAM para imagens** - preserva RAM interna
4. **Flash LED automático** - melhora qualidade
5. **Threshold adaptativo** - reduz falsos positivos
6. **Chunks MQTT** - transmissão robusta

## 🚨 Considerações de Segurança

### Acesso WiFi
- Configurar rede WiFi segura (WPA2/WPA3)
- Usar senhas fortes
- Considerar rede isolada para IoT

### MQTT
- Implementar autenticação se necessário
- Criptografia TLS para dados sensíveis
- Firewall para broker MQTT

### Dados
- Imagens podem conter informações sensíveis
- Implementar rotação de dados
- Backup seguro se necessário

---

## 📞 Suporte

Para dúvidas ou problemas:

**Gabriel Passos de Oliveira**  
Email: gabriel.passos@unesp.br  
IGCE/UNESP - Projeto de Iniciação Científica 2025

**Documentação de Referência:**
- [ESP32-CAM Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-cam_datasheet_en.pdf)
- [ESP32-Camera Component](https://github.com/espressif/esp32-camera)
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)

---

*Guia atualizado em Janeiro 2025 - Versão ESP32-CAM Real* 
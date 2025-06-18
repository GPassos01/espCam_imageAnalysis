# 📷 Manual ESP32-CAM - Hardware e Especificações

**Projeto de Iniciação Científica**  
**Gabriel Passos de Oliveira - IGCE/UNESP**  
**Versão 1.0 - Janeiro 2025**

---

## 1. Visão Geral

Manual técnico do módulo ESP32-CAM AI-Thinker utilizado no sistema de monitoramento de enchentes. Este documento foca nas especificações de hardware, pinout e características específicas da placa.

## 2. Especificações do Hardware

### 2.1 Componentes Principais

| Componente | Especificação | Detalhes |
|------------|---------------|----------|
| **Microcontrolador** | ESP32-S | Dual Core Xtensa LX6 @ 240MHz |
| **Memória RAM** | 520KB SRAM | ~320KB disponível para aplicação |
| **Memória PSRAM** | 4MB | Essencial para buffers de imagem |
| **Flash** | 4MB SPI | Particionável |
| **Câmera** | OV2640 | 2MP CMOS, até 1600x1200 |
| **Interface Câmera** | DVP | 8-bit parallel |
| **WiFi** | 802.11 b/g/n | 2.4GHz apenas |
| **Bluetooth** | 4.2 BR/EDR + BLE | Não usado neste projeto |
| **Antena** | PCB + u.FL | Conector para externa |

### 2.2 Características Elétricas

| Parâmetro | Min | Típico | Max | Unidade |
|-----------|-----|--------|-----|---------|
| Tensão de operação | 4.8 | 5.0 | 5.5 | V |
| Corrente (idle) | - | 80 | - | mA |
| Corrente (WiFi TX) | - | 240 | 400 | mA |
| Corrente (flash LED) | - | - | 480 | mA |
| Temperatura operação | -20 | 25 | 70 | °C |

## 3. Mapa de Pinos

### 3.1 Diagrama da Placa

```
┌─────────────────────────────────────┐
│         ESP32-CAM AI-Thinker        │
│                                     │
│  ANT                          5V ───│ Alimentação
│   ┌─┐                        GND ───│ Terra
│   └─┘                        IO12 ──│ Disponível*
│                              IO13 ──│ Disponível*
│ ┌─────┐                      IO15 ──│ Disponível*
│ │     │                      IO14 ──│ Disponível*
│ │ CAM │                      IO2 ───│ LED interno
│ │     │                      IO4 ───│ LED Flash
│ └─────┘                      IO16 ──│ PSRAM CS
│                              VCC ───│ 3.3V out
│                              U0R ───│ UART RX
│ [RESET]                      U0T ───│ UART TX
│                              GND ───│ Terra
│                              IO0 ───│ Boot/Flash
└─────────────────────────────────────┘

* GPIOs com limitações (ver seção 3.3)
```

### 3.2 Pinos da Câmera OV2640

| Função | GPIO | Direção | Descrição |
|--------|------|---------|-----------|
| PWDN | 32 | OUT | Power down (1=desliga) |
| RESET | -1 | - | Não conectado |
| XCLK | 0 | OUT | Clock 20MHz para câmera |
| SIOD | 26 | I/O | I2C Data (SDA) |
| SIOC | 27 | OUT | I2C Clock (SCL) |
| D7 | 35 | IN | Data bit 7 (MSB) |
| D6 | 34 | IN | Data bit 6 |
| D5 | 39 | IN | Data bit 5 |
| D4 | 36 | IN | Data bit 4 |
| D3 | 21 | IN | Data bit 3 |
| D2 | 19 | IN | Data bit 2 |
| D1 | 18 | IN | Data bit 1 |
| D0 | 5 | IN | Data bit 0 (LSB) |
| VSYNC | 25 | IN | Sincronização vertical |
| HREF | 23 | IN | Referência horizontal |
| PCLK | 22 | IN | Pixel clock |

### 3.3 GPIOs Disponíveis e Limitações

| GPIO | Status | Limitações |
|------|--------|------------|
| 0 | Usado (XCLK) | Boot strapping pin |
| 1 | TX0 | Debug serial |
| 2 | LED interno | Boot strapping pin |
| 3 | RX0 | Debug serial |
| 4 | LED Flash | Pode ser reutilizado |
| 12 | Livre | Boot strapping pin* |
| 13 | Livre | - |
| 14 | Livre | - |
| 15 | Livre | Boot strapping pin* |
| 16 | Usado (PSRAM) | Não disponível |

*Boot strapping pins: Cuidado ao usar durante boot

## 4. Módulo Câmera OV2640

### 4.1 Especificações do Sensor

- **Resolução**: 2 Megapixels (1600x1200)
- **Tamanho do sensor**: 1/4"
- **Pixel size**: 2.2μm x 2.2μm
- **Sensibilidade**: 0.6V/lux-sec
- **Dynamic range**: 50dB
- **Max frame rate**: 15fps @ UXGA, 30fps @ SVGA

### 4.2 Modos de Operação Suportados

| Formato | Resolução | FPS Max | Uso no Projeto |
|---------|-----------|---------|----------------|
| UXGA | 1600x1200 | 15 | Não usado |
| SXGA | 1280x1024 | 15 | Não usado |
| XGA | 1024x768 | 15 | Não usado |
| SVGA | 800x600 | 30 | Não usado |
| VGA | 640x480 | 30 | Não usado |
| **QVGA** | **320x240** | **30** | **✓ Usado** |
| QQVGA | 160x120 | 30 | Não usado |

### 4.3 Formatos de Saída

- **JPEG**: Compressão hardware (usado no projeto)
- **RGB565**: 16-bit por pixel
- **YUV422**: 16-bit por pixel
- **Grayscale**: 8-bit por pixel

## 5. Alimentação e Consumo

### 5.1 Requisitos de Alimentação

⚠️ **IMPORTANTE**: A ESP32-CAM requer fonte de alimentação robusta!

- **Tensão**: 5V ±5% (regulador onboard para 3.3V)
- **Corrente mínima**: 500mA
- **Corrente recomendada**: 2A
- **Capacitor de bypass**: 100-470μF próximo ao VCC

### 5.2 Perfil de Consumo

```
Estado                 Corrente    Potência
─────────────────────────────────────────────
Deep Sleep            10mA        0.05W
Idle (WiFi off)       80mA        0.40W
WiFi connected        120mA       0.60W
Capturando imagem     180mA       0.90W
WiFi TX               240mA       1.20W
WiFi TX + Captura     320mA       1.60W
Flash LED ligado      +240mA      +1.20W
```

### 5.3 Problemas Comuns de Alimentação

| Sintoma | Causa | Solução |
|---------|-------|---------|
| Brownout detector | Fonte inadequada | Usar fonte 5V/2A |
| Reset durante TX | Queda de tensão | Adicionar capacitor |
| Câmera falha | Corrente insuficiente | Fonte externa |

## 6. Conexões para Programação

### 6.1 Esquema FTDI

```
ESP32-CAM          FTDI/USB-Serial
─────────          ───────────────
5V      ─────────  5V (ou VCC)
GND     ─────────  GND
U0R     ─────────  TX
U0T     ─────────  RX
IO0     ────┐
GND     ────┘      (jumper para flash)
```

### 6.2 Procedimento de Upload

1. **Conectar** jumper IO0-GND
2. **Conectar** FTDI ao computador
3. **Pressionar** botão RESET
4. **Upload** do firmware
5. **Remover** jumper IO0-GND
6. **Pressionar** RESET novamente

## 7. Considerações de Design

### 7.1 Layout PCB

- Manter antena WiFi longe de metais
- Área de cobre sob antena deve ser removida
- Trilhas de alimentação largas (>1mm)
- Capacitores próximos aos pinos de alimentação

### 7.2 Dissipação Térmica

- ESP32 pode aquecer durante operação contínua
- Considerar dissipador para aplicações 24/7
- Manter boa ventilação no gabinete
- Temperatura máxima do chip: 125°C

### 7.3 Interferência Eletromagnética

- Câmera sensível a EMI
- Usar cabos curtos quando possível
- Blindagem pode ser necessária em ambientes ruidosos
- Manter distância de fontes chaveadas

## 8. Troubleshooting de Hardware

### 8.1 Diagnóstico Rápido

| LED | Estado | Significado |
|-----|--------|-------------|
| Vermelho (GPIO33) | Aceso | Placa alimentada |
| Flash (GPIO4) | Piscando | Atividade/Captura |
| Azul (GPIO2) | Variável | Definido por software |

### 8.2 Testes Básicos

```bash
# Verificar comunicação serial
screen /dev/ttyUSB0 115200

# Verificar boot
# Deve mostrar mensagens do bootloader

# Testar câmera (após flash)
# LOG: Camera probe success
```

### 8.3 Falhas Comuns

1. **"Camera probe failed"**
   - Verificar alimentação
   - Confirmar PSRAM habilitado
   - Testar outra placa

2. **Boot loop**
   - Fonte inadequada
   - Flash corrompido
   - GPIO0 ainda em GND

3. **WiFi não conecta**
   - Antena danificada
   - Usar antena externa
   - Verificar se é 2.4GHz

## 9. Otimizações de Hardware

### 9.1 Melhorar Alcance WiFi

- Soldar conector u.FL
- Usar antena externa 2.4GHz
- Posicionar longe de obstáculos
- Ganho típico: +3 a +5dBi

### 9.2 Reduzir Consumo

- Desabilitar LED flash quando não usado
- Usar deep sleep entre capturas
- Reduzir potência TX WiFi se possível
- Desligar Bluetooth (não usado)

## 10. Referências

- [ESP32-CAM Schematic](https://github.com/SeeedDocument/forum_doc/raw/master/reg/ESP32_CAM_V1.6.pdf)
- [OV2640 Datasheet](http://www.ovt.com/download_document.php?type=sensor&sensorid=80)
- [ESP32 Hardware Design Guidelines](https://www.espressif.com/sites/default/files/documentation/esp32_hardware_design_guidelines_en.pdf)

---

Para instalação e configuração de software, consulte o [Guia de Instalação](INSTALACAO.md).  
Para protocolo de comunicação, veja [API MQTT](API_MQTT.md).

**Autor:** Gabriel Passos de Oliveira  
**Email:** gabriel.passos@unesp.br  
**IGCE/UNESP** - Janeiro 2025 
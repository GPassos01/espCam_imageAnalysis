# 📋 Documentação Técnica - Sistema de Monitoramento de Enchentes ESP32-CAM

**Versão:** 1.0  
**Data:** Janeiro 2025  
**Autor:** Gabriel Passos de Oliveira - IGCE/UNESP

---

## 1. Visão Geral do Sistema

### 1.1 Descrição
Sistema embarcado para monitoramento contínuo de níveis fluviais utilizando processamento local de imagens. Detecta mudanças através da análise comparativa de frames JPEG capturados por ESP32-CAM, otimizado para ambientes com recursos limitados.

### 1.2 Objetivos Técnicos
- Reduzir consumo de dados móveis em 95% vs. streaming contínuo
- Detectar mudanças significativas com latência < 2s
- Operar com disponibilidade > 99%
- Manter consumo energético < 1.5W médio

### 1.3 Arquitetura em Camadas

```
┌─────────────────────────┐
│   Camada de Captura     │
│  ├─ OV2640 (640x480)    │
│  └─ Frame Buffer 8MB PSRAM │
├─────────────────────────┤
│   Camada de Análise     │
│  ├─ Algoritmo JPEG Size │
│  └─ Threshold Logic     │
├─────────────────────────┤
│   Camada de Comunicação │
│  ├─ MQTT Client (QoS1)  │
│  └─ WiFi 802.11n 2.4GHz │
└─────────────────────────┘
```

---

## 2. Arquitetura de Software

### 2.1 Módulos do Firmware

```
esp32/main/
├── main.c (345 linhas)
│   └── Coordenação de tarefas e loop principal
├── config.h (86 linhas)
│   └── Configurações centralizadas
└── model/
    ├── compare.c/h
    │   └── Algoritmo de detecção por tamanho JPEG
    ├── init_hw.c/h
    │   └── Inicialização câmera e GPIO
    ├── init_net.c/h
    │   └── Gestão WiFi e MQTT com reconexão
    ├── mqtt_send.c/h
    │   └── Serialização e envio de dados
    └── wifi_sniffer.c/h
        └── Análise de tráfego promíscuo
```

### 2.2 Fluxo de Execução Principal

```c
monitoring_task() {
    while(1) {
        // A cada 15 segundos
        fb = esp_camera_fb_get();
        diff = calculate_image_difference(fb, previous);
        
        if (diff >= CHANGE_THRESHOLD) {
            mqtt_send_monitoring_data();
            if (diff >= ALERT_THRESHOLD) {
                mqtt_send_alert();
                mqtt_send_image();
            }
        }
        
        esp_camera_fb_return(fb);
        vTaskDelay(15000);
    }
}
```

---

## 3. Algoritmo de Detecção

### 3.1 Princípio de Funcionamento

O algoritmo explora a propriedade de que mudanças visuais significativas alteram a compressibilidade JPEG:

```c
float calculate_image_difference(fb1, fb2) {
    size_diff = abs(fb1->len - fb2->len);
    avg_size = (fb1->len + fb2->len) / 2.0f;
    variation = (size_diff / avg_size) * 100.0f;
    
    if (variation < 0.5f) return 0.0f;      // Ruído
    if (variation > 50.0f) return 50.0f;    // Saturação
    return variation;
}
```

### 3.2 Calibração de Thresholds

| Threshold | Valor | Justificativa |
|-----------|-------|---------------|
| NOISE_FLOOR | 0.5% | Variações normais de compressão |
| CHANGE_THRESHOLD | 1.0% | Menor mudança significativa |
| ALERT_THRESHOLD | 8.0% | Mudança crítica (enchente) |
| SATURATION | 50.0% | Limite superior |

### 3.3 Performance

- **Complexidade**: O(1) - comparação direta
- **Tempo médio**: 50ms por análise
- **Uso de CPU**: 15-20% durante análise
- **Precisão**: 92% em testes controlados

---

## 4. Especificações de Hardware

### 4.1 Consumo de Recursos

```yaml
CPU:
  - Idle: 3-5% @ 240MHz
  - Captura: 25-30%
  - Análise: 15-20%
  - Transmissão: 10-15%

Memória:
  - Heap: 180KB/320KB (56%)
  - PSRAM: 2.4MB/8MB (30%) - Configuração premium
  - Stack: 8KB (monitoring_task)

Energia:
  - Sleep: 20mA @ 5V (0.1W)
  - Idle: 80mA @ 5V (0.4W)
  - Ativo: 240mA @ 5V (1.2W)
  - Pico: 380mA @ 5V (1.9W)
```

### 4.2 Pinout Utilizado

Consulte [Manual ESP32-CAM](ESP32-CAM_README.md#conexões-da-câmera-ov2640) para pinout completo.

---

## 5. Banco de Dados

### 5.1 Schema Otimizado

```sql
-- Configurações SQLite
PRAGMA journal_mode = WAL;
PRAGMA synchronous = NORMAL;
PRAGMA cache_size = 10000;

-- Índices para queries frequentes
CREATE INDEX idx_timestamp ON table(timestamp);
```

### 5.2 Estimativa de Armazenamento

| Período | Volume de Dados |
|---------|-----------------|
| 1 dia | ~1.3 MB |
| 1 semana | ~9.1 MB |
| 1 mês | ~39 MB |
| 1 ano | ~475 MB |

---

## 6. Otimizações Implementadas

### 6.1 Firmware
1. **Double buffering**: Captura paralela
2. **JPEG nativo**: Sem conversão de formato
3. **PSRAM 8MB exclusivo**: Para buffers avançados e histórico
4. **Watchdog timer**: Recuperação automática

### 6.2 Comunicação
1. **JSON compacto**: `cJSON_PrintUnformatted()`
2. **Base64 inline**: Reduz overhead
3. **QoS seletivo**: 0 para status, 1 para dados
4. **Keep-alive otimizado**: 60 segundos

### 6.3 Servidor
1. **WAL mode**: Write-ahead logging
2. **Batch inserts**: Quando aplicável
3. **Connection pooling**: Para MQTT
4. **Async I/O**: Non-blocking operations

---

## 7. Métricas de Performance

### 7.1 Benchmarks

| Operação | Tempo | CPU | Observações |
|----------|-------|-----|-------------|
| Boot completo | 5-7s | 100% | Inclui WiFi+MQTT |
| Captura frame | 150ms | 25% | DMA transfer |
| Análise | 50ms | 20% | Comparação |
| Base64 encode | 80ms | 25% | Para 8KB |
| MQTT publish | 200ms | 15% | QoS 1 |

### 7.2 Throughput

- **Taxa máxima**: 4 fps (limitado por câmera)
- **Taxa operacional**: 0.067 fps (15s intervalo)
- **Dados médios**: 1.6-2.4 KB/s
- **Pico com imagem**: 40-60 KB/s

---

## 8. Limitações e Trade-offs

### 8.1 Limitações Técnicas
- Algoritmo sensível a mudanças de iluminação
- Detecção global apenas (não localizada)
- Dependência de infraestrutura WiFi
- Resolução VGA otimizada para 8MB PSRAM

### 8.2 Trade-offs de Design
- **Simplicidade vs. Precisão**: Algoritmo simples mas robusto
- **Latência vs. Consumo**: 15s otimiza bateria
- **Resolução vs. Performance**: VGA com 8MB PSRAM
- **Local vs. Cloud**: Processamento embarcado

---

## 9. Manutenção

### 9.1 Logs Críticos

```
I IMG_MONITOR: 📸 Capturando foto...
I IMG_MONITOR: 📷 Foto capturada: 6052 bytes
I IMG_MONITOR: 🔍 Diferença: 2.3%
E IMG_MONITOR: Camera probe failed
W IMG_MONITOR: Heap baixa: 45KB
```

### 9.2 Monitoramento de Saúde

- Heap livre > 50KB
- PSRAM livre > 2MB (de 8MB total)
- WiFi RSSI > -70dBm
- MQTT reconnects < 5/hora
- Uptime > 24h

---

## 10. Roadmap Técnico

### Curto Prazo (3 meses)
- [ ] Otimização de consumo energético
- [ ] Melhoria do algoritmo com histograma
- [ ] Dashboard web básico

### Médio Prazo (6 meses)
- [ ] Suporte multi-câmera
- [ ] Protocolo LoRaWAN
- [ ] Edge AI com TensorFlow Lite

### Longo Prazo (12 meses)
- [ ] Alimentação solar
- [ ] Mesh networking
- [ ] Integração com sistemas de alerta

---

## 📚 Referências Técnicas

1. **ESP32 Technical Reference Manual** - Espressif Systems
2. **OV2640 Camera Module Datasheet** - OmniVision
3. **JPEG Compression Standard** - ITU-T T.81
4. **MQTT Protocol Specification v3.1.1** - OASIS

---

**📧 Contato:** gabriel.passos@unesp.br  
**🏫 IGCE/UNESP** - Iniciação Científica 2025 
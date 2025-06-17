# 📋 Documentação Técnica - Sistema ESP32-CAM

## **Monitoramento por Comparação de Imagens**
**Gabriel Passos - IGCE/UNESP 2025**

---

## 🏗️ **Arquitetura**

```
ESP32-CAM (C/C++)     ←→     Servidor Python
     ↓                            ↓
Captura + Análise               MQTT + SQLite
     ↓                            ↓
MQTT (15s)                   Relatórios PDF
```

---

## 🔧 **ESP32-CAM - Estrutura**

```
esp32/main/
├── main.c              # Sistema principal de monitoramento
├── config.h            # Configurações centralizadas
└── model/
    ├── compare.c/h     # Algoritmo de comparação (30 pontos)
    ├── init_hw.c/h     # Hardware: câmera + PSRAM
    ├── init_net.c/h    # WiFi + MQTT
    ├── mqtt_send.c/h   # Envio estruturado MQTT
    └── wifi_sniffer.c/h # WiFi packet sniffer para medição de tráfego
```

---

## 🎯 **Componentes Principais**

### **Sistema Principal (`main.c`)**
- **Ciclo:** Captura (15s) → Compara → MQTT → Estatísticas (5min)
- **Thresholds:** 10% mudança, 30% alerta
- **Memória:** PSRAM para imagens, heap para estruturas
- **Flash:** Desabilitado para economia de energia

### **Algoritmo (`compare.c`)**
- **30 pontos** de amostragem distribuídos
- **Métricas:** 60% conteúdo + 25% tamanho + 15% mudanças grandes
- **Filtros:** Redução de ruído, amplificação de grandes mudanças
- **Validação:** Rigorosa para evitar falsos positivos

### **MQTT (`mqtt_send.c`)**
- **5 tópicos:** data, alert, image/metadata, image/data/{ts}/{offset}, sniffer/stats
- **Chunks:** 1KB para imagens grandes
- **QoS:** 1 para dados críticos, 0 para chunks
- **Reconnect:** Automático com fallbacks

### **WiFi Sniffer (`wifi_sniffer.c`)**
- **Modo promíscuo:** Captura pacotes WiFi em tempo real
- **Filtro MQTT:** Identifica tráfego do próprio ESP32
- **Métricas:** Packets/bytes totais e específicos MQTT
- **Throughput:** Medição de largura de banda por transmissão de imagem

---

## 🐍 **Servidor Python - Estrutura**

```
server/
├── ic_monitor.py           # Monitor MQTT + SQLite
├── generate_report.py      # Relatórios PDF
└── monitoring_data.db      # Banco (3 tabelas)
```

### **Monitor (`ic_monitor.py`)**
- **5 handlers:** Dados, alertas, metadados, chunks de imagem, stats sniffer
- **SQLite:** 4 tabelas (readings, alerts, images, sniffer_stats)
- **Reconstitui:** Imagens via chunks ordenados
- **Estatísticas:** Tempo real a cada 60s + throughput MQTT

### **Relatórios (`generate_report.py`)**
- **Seções:** Resumo, estatísticas, leituras, alertas, imagens
- **Análise:** Últimas 24h, distribuição de alertas
- **Output:** PDF profissional com tabelas

---

## 📡 **Protocolo MQTT**

### **Tópicos:**
```
monitoring/data                        # Dados principais
monitoring/alert                       # Alertas críticos  
monitoring/image/metadata              # Info da imagem
monitoring/image/data/{ts}/{offset}    # Chunks 1KB
```

### **Dados (`monitoring/data`):**
```json
{
  "timestamp": 1704067200,
  "difference": 0.234,
  "image_size": 45678,
  "width": 320, "height": 240, "format": 4,
  "location": "monitoring_esp32cam",
  "mode": "image_comparison"
}
```

---

## 🗄️ **Banco SQLite**

### **4 Tabelas:**
```sql
monitoring_readings: id, timestamp, image_size, difference, width, height, format, location, mode
alerts: id, timestamp, alert_type, difference, location, mode  
received_images: id, timestamp, filename, file_size, width, height, format, reason, device
sniffer_stats: id, timestamp, total_packets, mqtt_packets, total_bytes, mqtt_bytes, image_packets, image_bytes, uptime, channel, device
```

---

## ⚙️ **Configurações Críticas**

### **ESP32 (`config.h`):**
```c
#define CAPTURE_INTERVAL_MS     15000    // 15 segundos
#define CHANGE_THRESHOLD       0.10f     // 10% mudança
#define ALERT_THRESHOLD        0.30f     // 30% alerta
#define JPEG_QUALITY           10        // Qualidade otimizada
```

### **Python:**
```python
MQTT_BROKER = "192.168.1.29"    # Broker MQTT
DATABASE_FILE = "monitoring_data.db"
IMAGES_DIR = "received_images"
```

---

## 🚀 **Execução**

### **ESP32:**
```bash
cd esp32
idf.py build flash monitor
```

### **Servidor:**
```bash
cd server
python3 ic_monitor.py        # Terminal 1
python3 generate_report.py   # Terminal 2 (quando necessário)
```

---

## 📈 **Performance**

| Métrica | Valor |
|---------|-------|
| Intervalo captura | 15s |
| Tempo de análise | ~50-100ms |
| Tamanho por foto | 5-8 KB (QVGA JPEG Q10) |
| Chunks por imagem | 5-8 chunks de 1KB |
| Uso RAM ESP32 | ~200 KB heap + 4 MB PSRAM |
| Precisão algoritmo | 30 pontos distribuídos |

---

## 🛠️ **APIs Principais**

### **ESP32:**
```c
float calculate_image_difference(camera_frame_t *img1, camera_frame_t *img2);
esp_err_t mqtt_send_monitoring_data(float difference, uint32_t image_size, ...);
esp_err_t mqtt_send_alert(float difference, const char* alert_type, ...);
```

### **Python:**
```python
class ICImageMonitor:
    def start_monitoring(self)    # Inicia monitoramento
    def stop_monitoring(self)     # Para gracefully
    def print_statistics(self)    # Estatísticas tempo real

def generate_report() -> bool     # Gera relatório PDF
```

---

## 🔧 **Troubleshooting**

### **ESP32:**
- **Memória baixa:** Verificar PSRAM habilitado
- **Falha captura:** Fonte 5V/2A estável
- **MQTT falha:** Verificar credenciais e rede

### **Python:**
- **Chunks perdidos:** Verificar ordenação por offset
- **Banco travado:** sqlite3.connect(timeout=30)
- **Imagens corrompidas:** Validação de integridade

---

**📧 Contato:** gabriel.passos@unesp.br  
**🏫 IGCE/UNESP** - Iniciação Científica 2025 
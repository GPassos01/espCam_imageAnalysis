/**
 * Configurações Centralizadas - Sistema ESP32-CAM
 * 
 * Todas as configurações do sistema em um local único:
 * - Rede WiFi e MQTT
 * - Pinout ESP32-CAM AI-Thinker
 * - Parâmetros de captura e processamento
 * - Thresholds de detecção de mudanças
 * 
 * @author Gabriel Passos - UNESP 2025
 */
#ifndef CONFIG_H
#define CONFIG_H

// ===== REDE WiFi E MQTT =====
#define WIFI_SSID        "Steps 2.4G"      // Nome da rede WiFi 2.4GHz
#define WIFI_PASS        "h%8Ka4D&"        // Senha da rede
#define WIFI_MAXIMUM_RETRY  5               // Tentativas de reconexão WiFi
#define MQTT_BROKER_URI  "mqtt://192.168.1.29:1883"  // Broker MQTT local
#define MQTT_USERNAME    "gabriel"          // Usuário MQTT
#define MQTT_PASSWORD    "gabriel123"       // Senha MQTT

// ===== CONFIGURAÇÕES DE DISPOSITIVO =====
#define DEVICE_ID        "esp32_cam_001"

// ===== PINOUT ESP32-CAM AI-THINKER (NÃO ALTERAR) =====
#define CAM_PIN_PWDN    32    // Power Down da câmera
#define CAM_PIN_RESET   -1    // Reset (não conectado)
#define CAM_PIN_XCLK    0     // Clock master da câmera
#define CAM_PIN_SIOD    26    // I2C SDA (dados)
#define CAM_PIN_SIOC    27    // I2C SCL (clock)
#define CAM_PIN_D7      35    // Data bit 7
#define CAM_PIN_D6      34    // Data bit 6  
#define CAM_PIN_D5      39    // Data bit 5
#define CAM_PIN_D4      36    // Data bit 4
#define CAM_PIN_D3      21    // Data bit 3
#define CAM_PIN_D2      19    // Data bit 2
#define CAM_PIN_D1      18    // Data bit 1
#define CAM_PIN_D0      5     // Data bit 0
#define CAM_PIN_VSYNC   25    // Vertical sync
#define CAM_PIN_HREF    23    // Horizontal reference
#define CAM_PIN_PCLK    22    // Pixel clock
#define CAM_PIN_FLASH   4     // LED flash integrado (DESABILITADO)

// ===== PARÂMETROS DE CAPTURA =====
#define CAPTURE_INTERVAL_MS     15000    // Intervalo entre capturas (15 segundos)
#define STATUS_INTERVAL_MS      300000   // Intervalo para estatísticas (5 minutos)
#define IMAGE_WIDTH            320       // Largura QVGA
#define IMAGE_HEIGHT           240       // Altura QVGA
#define JPEG_QUALITY           10        // Qualidade JPEG 0-63 (menor=melhor)

// ===== ALGORITMO DE DETECÇÃO =====
#define CHANGE_THRESHOLD       0.10f     // 10% diferença mínima para mudança
#define ALERT_THRESHOLD        0.30f     // 30% diferença para alerta crítico

// ===== CONFIGURAÇÕES DE MQTT =====
#define SEND_IMAGE_ON_ALERT    true      // Enviar imagem completa em alertas
#define MAX_RETRY_ATTEMPTS     3         // Tentativas de reenvio MQTT

// ===== CONFIGURAÇÕES DO WIFI SNIFFER =====
#define SNIFFER_ENABLED        true      // Habilitar monitoramento de tráfego
#define SNIFFER_CHANNEL        8         // Canal WiFi para sniffing (8 = Steps 2.4G)
#define SNIFFER_STATS_INTERVAL 60       // Intervalo para estatísticas do sniffer (segundos)

#endif // CONFIG_H 
/**
 * @file config.h
 * @brief Configurações Centralizadas - Sistema de Monitoramento de Enchentes ESP32-CAM
 * 
 * Este arquivo centraliza todas as configurações do sistema de monitoramento
 * de enchentes baseado em análise de imagens com ESP32-CAM.
 * 
 * @version 1.0
 * @date 2025
 * @author Gabriel Passos de Oliveira - IGCE/UNESP
 * 
 * @copyright MIT License
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===== REDE WiFi E MQTT =====
#define WIFI_SSID        "Steps 2.4G"      // Nome da rede WiFi 2.4GHz
#define WIFI_PASS        "h%8Ka4D&"        // Senha da rede
#define WIFI_MAXIMUM_RETRY  5               // Tentativas de reconexão WiFi
#define MQTT_BROKER_URI  "mqtt://192.168.1.48:1883"  // Broker MQTT local
#define MQTT_USERNAME    ""                 // Usuário MQTT
#define MQTT_PASSWORD    ""                 // Senha MQTT
#define MQTT_KEEPALIVE   60                 // Keepalive MQTT em segundos
#define MQTT_RECONNECT_DELAY_MS 5000        // Delay entre tentativas de reconexão

// ===== CONFIGURAÇÕES DE DISPOSITIVO =====
#define DEVICE_ID        "esp32_cam_001"
#define DEVICE_NAME      "ESP32-CAM Monitor"

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
#define JPEG_QUALITY           12        // Qualidade JPEG 0-63 (menor=melhor, otimizado para performance)
#define FRAMESIZE             FRAMESIZE_QVGA  // Tamanho do frame
#define PIXEL_FORMAT          PIXFORMAT_JPEG  // Formato do pixel

// ===== ALGORITMO DE DETECÇÃO =====
#define CHANGE_THRESHOLD       1.0f      // 1.0% diferença mínima para mudança (otimizado para reduzir ruído)
#define ALERT_THRESHOLD        8.0f      // 8.0% diferença para alerta crítico (otimizado para eventos reais)
#define MOTION_DETECTION_ENABLED true     // Habilitar detecção de movimento

// ===== CONFIGURAÇÕES DE MQTT =====
#define SEND_IMAGE_ON_ALERT    true      // Enviar imagem completa em alertas
#define MAX_RETRY_ATTEMPTS     3         // Tentativas de reenvio MQTT
#define MQTT_TOPIC_BASE        "esp32cam" // Tópico base MQTT
#define MQTT_TOPIC_ALERT       "alert"    // Tópico para alertas
#define MQTT_TOPIC_STATUS      "status"   // Tópico para status
#define MQTT_TOPIC_IMAGE       "image"    // Tópico para imagens
#define MQTT_TOPIC_STATS       "stats"    // Tópico para estatísticas

// ===== CONFIGURAÇÕES DO WIFI SNIFFER =====
#define SNIFFER_ENABLED        true      // Habilitar monitoramento de tráfego
#define SNIFFER_CHANNEL        8         // Canal WiFi para sniffing (8 = Steps 2.4G)
#define SNIFFER_STATS_INTERVAL 60        // Intervalo para estatísticas do sniffer (segundos)
#define SNIFFER_MAX_PACKETS    1000      // Máximo de pacotes para análise
#define SNIFFER_BUFFER_SIZE    4096      // Tamanho do buffer do sniffer

// ===== CONFIGURAÇÕES DE MEMÓRIA =====
#define PSRAM_ENABLED         true       // Habilitar uso de PSRAM
#define JPEG_BUFFER_SIZE      (IMAGE_WIDTH * IMAGE_HEIGHT / 8)  // Buffer para JPEG
#define MAX_IMAGE_SIZE        32768      // Tamanho máximo da imagem em bytes

#endif // CONFIG_H 
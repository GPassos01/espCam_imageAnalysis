/**
 * @file config.h
 * @brief Configurações Centralizadas - Sistema de Monitoramento ESP32-CAM
 * 
 * Configurações otimizadas para HVGA (480x320) com qualidade premium
 * 
 * @version 2.0 - Otimizada para testes finais
 * @author Gabriel Passos de Oliveira - IGCE/UNESP
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===== REDE WiFi E MQTT =====
#define WIFI_SSID        "Cosmos_2.4G"       // Nome da rede WiFi 2.4GHz
#define WIFI_PASS        "31471189"         // Senha da rede
#define WIFI_MAXIMUM_RETRY  5               // Tentativas de reconexão WiFi
#define MQTT_BROKER_URI  "mqtt://192.168.1.16:1883"  // Broker MQTT local
#define MQTT_USERNAME    ""                 // Usuário MQTT (vazio)
#define MQTT_PASSWORD    ""                 // Senha MQTT (vazia)

// ===== IDENTIFICAÇÃO DO DISPOSITIVO =====
#define DEVICE_ID        "esp32_cam_001"
#define DEVICE_NAME      "ESP32-CAM Monitor HVGA"

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
#define CAM_PIN_FLASH   4     // LED flash integrado

// ===== CONFIGURAÇÕES DE CAPTURA =====
#define CAPTURE_INTERVAL_MS     15000    // Intervalo entre capturas (15 segundos)

// ===== CONFIGURAÇÕES DE RESOLUÇÃO OTIMIZADA PARA QUALIDADE =====
#define IMAGE_WIDTH            480       // Largura HVGA (otimizada para qualidade)
#define IMAGE_HEIGHT           320       // Altura HVGA (50% dos pixels, melhor qualidade)
#define JPEG_QUALITY           5         // Qualidade JPEG premium (0-63, menor=melhor)
#define FRAMESIZE             FRAMESIZE_HVGA  // Tamanho do frame HVGA
#define PIXEL_FORMAT          PIXFORMAT_JPEG  // Formato do pixel

// ===== ALGORITMO DE DETECÇÃO OTIMIZADO =====
#define CHANGE_THRESHOLD       3.0f      // 3.0% diferença mínima para mudança
#define ALERT_THRESHOLD        12.0f     // 12.0% diferença para alerta crítico

// ===== ANÁLISE AVANÇADA COM PSRAM =====
#define ENABLE_HISTORY_BUFFER  true      // Buffer de histórico de imagens
#define HISTORY_BUFFER_SIZE    3         // Número de imagens no histórico (otimizado)
#define ENABLE_ADVANCED_ANALYSIS true    // Análise avançada de padrões

// ===== CONFIGURAÇÕES DE MQTT =====
#define SEND_IMAGE_ON_ALERT    true      // Enviar imagem completa em alertas
#define MQTT_TOPIC_BASE        "esp32cam" // Tópico base MQTT
#define MQTT_TOPIC_ALERT       "alert"    // Tópico para alertas
#define MQTT_TOPIC_STATUS      "status"   // Tópico para status
#define MQTT_TOPIC_IMAGE       "image"    // Tópico para imagens

// ===== CONFIGURAÇÕES DO WIFI SNIFFER =====
#define SNIFFER_ENABLED        true      // Habilitar monitoramento de tráfego
#define SNIFFER_CHANNEL        8         // Canal WiFi para sniffing
#define SNIFFER_STATS_INTERVAL 60        // Intervalo para estatísticas (segundos)

// ===== CONFIGURAÇÕES DE MEMÓRIA OTIMIZADA =====
#define MAX_IMAGE_SIZE        71680      // 70KB máximo por imagem HVGA (otimizado)
#define HISTORY_BUFFER_TOTAL  (MAX_IMAGE_SIZE * HISTORY_BUFFER_SIZE)  // ~210KB para histórico

#endif // CONFIG_H 
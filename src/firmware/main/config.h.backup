/**
 * @file config.h
 * @brief Configurações Centralizadas - Sistema de Monitoramento ESP32-CAM
 * 
 * Configurações otimizadas para HVGA (480x320) com qualidade premium
 * Sistema com duas versões: INTELIGENTE (com comparação) e SIMPLES (sem comparação)
 * 
 * @version 2.0 - Otimizada para testes científicos comparativos
 * @author Gabriel Passos de Oliveira - IGCE/UNESP
 */

#ifndef CONFIG_H
#define CONFIG_H

// =====================================================
// CONFIGURAÇÕES DE REDE E COMUNICAÇÃO
// =====================================================
#define WIFI_SSID        "Racho Floresta"       // Nome da rede WiFi 2.4GHz
#define WIFI_PASS        "jujumax1969"         // Senha da rede
#define WIFI_MAXIMUM_RETRY  5               // Tentativas de reconexão WiFi

// MQTT - Auto-detectado pelo script find_mosquitto_ip.sh
#define MQTT_BROKER_URI  "mqtt://192.168.1.38:1883"  // Auto-detectado
#define MQTT_USERNAME    ""                 // Usuário MQTT (vazio = sem auth)
#define MQTT_PASSWORD    ""                 // Senha MQTT (vazia = sem auth)

// =====================================================
// IDENTIFICAÇÃO DO DISPOSITIVO
// =====================================================
#define DEVICE_ID        "esp32_cam_001"
#define DEVICE_NAME      "ESP32-CAM Monitor HVGA"

// =====================================================
// PINOUT ESP32-CAM AI-THINKER (HARDWARE FIXO)
// =====================================================
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

// =====================================================
// CONFIGURAÇÕES DE CAPTURA E QUALIDADE
// =====================================================
#define CAPTURE_INTERVAL_MS     15000    // Intervalo entre capturas (15 segundos)

// Resolução otimizada para qualidade vs. performance
#define IMAGE_WIDTH            480       // Largura HVGA
#define IMAGE_HEIGHT           320       // Altura HVGA
#define JPEG_QUALITY           10        // Qualidade JPEG balanceada (0-63, menor=melhor) - ajustado para evitar out-of-memory
#define FRAMESIZE             FRAMESIZE_HVGA  // Tamanho do frame
#define PIXEL_FORMAT          PIXFORMAT_JPEG  // Formato do pixel

// =====================================================
// ALGORITMO DE DETECÇÃO (VERSÃO INTELIGENTE)
// =====================================================
#define CHANGE_THRESHOLD       8.0f      // 8% diferença mínima para mudança
#define ALERT_THRESHOLD        15.0f     // 15% diferença para alerta crítico

// =====================================================
// DETECÇÃO INTELIGENTE AVANÇADA (VERSÃO PRINCIPAL)
// =====================================================
#define ENHANCED_DETECTION        true   // Algoritmo principal de detecção robusta
#define ENHANCED_NOISE_FILTER     true   // Filtro de ruído multi-camada
#define MULTI_FRAME_VALIDATION    true   // Validação temporal inteligente
#define MIN_CONSECUTIVE_CHANGES   3      // Mudanças consecutivas para confirmar validação
#define NOISE_REDUCTION_PASSES    2      // Passadas de redução de ruído

// =====================================================
// CONFIGURAÇÕES DE ESTABILIDADE OPERACIONAL
// =====================================================
#define DISABLE_ADAPTIVE_SETTINGS  true // Sistema estável sem adaptações automáticas
#define DISABLE_PERIODIC_WARMUP    true // Warm-ups desabilitados para estabilidade
#define DISABLE_ANOMALY_DETECTION  true // Detecção de anomalias desabilitada (não necessária)
#define DISABLE_AUTO_REF_UPDATE    false // PERMITIR atualização periódica da referência
#define STATIC_REFERENCE_MODE     false  // PERMITIR referência adaptativa para evitar drift

// =====================================================
// ANÁLISE AVANÇADA COM PSRAM
// =====================================================
#define ENABLE_HISTORY_BUFFER  true      // Buffer de histórico para análise temporal
#define HISTORY_BUFFER_SIZE    3         // Número de imagens no histórico (otimizado)
#define ENABLE_ADVANCED_ANALYSIS false   // Análise avançada desabilitada (não necessária)

// =====================================================
// CONFIGURAÇÕES DE MQTT E TÓPICOS
// =====================================================
#define SEND_IMAGE_ON_ALERT    true      // Enviar imagem completa em alertas
#define MQTT_TOPIC_BASE        "esp32cam" // Tópico base MQTT
#define MQTT_TOPIC_ALERT       "alert"    // Tópico para alertas
#define MQTT_TOPIC_STATUS      "status"   // Tópico para status
#define MQTT_TOPIC_IMAGE       "image"    // Tópico para imagens

// =====================================================
// MONITORAMENTO DE REDE (WIFI SNIFFER)
// =====================================================
#define SNIFFER_ENABLED        true      // Habilitar monitoramento de tráfego
#define SNIFFER_CHANNEL        8         // Canal WiFi para sniffing
#define SNIFFER_STATS_INTERVAL 60        // Intervalo para estatísticas (segundos)

// =====================================================
// OTIMIZAÇÕES DE MEMÓRIA
// =====================================================
#define MAX_IMAGE_SIZE        71680      // 70KB máximo por imagem HVGA
#define HISTORY_BUFFER_TOTAL  (MAX_IMAGE_SIZE * HISTORY_BUFFER_SIZE)  // ~210KB para histórico

#endif // CONFIG_H 
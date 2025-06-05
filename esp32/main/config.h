#ifndef CONFIG_H
#define CONFIG_H

// ===== CONFIGURAÇÕES DE REDE =====
#define WIFI_SSID        "Steps 2.4G"
#define WIFI_PASS        "h%8Ka4D&"
#define WIFI_MAXIMUM_RETRY  5
#define MQTT_BROKER_URI  "mqtt://192.168.1.2:1883"
#define MQTT_USERNAME    ""
#define MQTT_PASSWORD    ""

// ===== CONFIGURAÇÕES DA ESP32-CAM =====
#define CAM_PIN_PWDN    32
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK    0
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27
#define CAM_PIN_D7      35
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      21
#define CAM_PIN_D2      19
#define CAM_PIN_D1      18
#define CAM_PIN_D0      5
#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22
#define CAM_PIN_FLASH   4

// ===== CONFIGURAÇÕES DE DISPOSITIVO =====
#define DEVICE_ID        "ESP32_IC_001"
#define TANK_HEIGHT_CM   100.0f    // Altura do tanque em cm (configurar conforme setup)

// ===== CONFIGURAÇÕES DE SENSORES =====
#define HC_SR04_TRIG_PIN    GPIO_NUM_12  // GPIO disponível para TRIG
#define HC_SR04_ECHO_PIN    GPIO_NUM_13  // GPIO disponível para ECHO

// ===== CONFIGURAÇÕES DE CAPTURA =====
#define CAPTURE_INTERVAL_MS     60000    // 1 minuto (reduzido para processamento local)
#define STATUS_INTERVAL_MS      300000   // 5 minutos para status
#define IMAGE_WIDTH            320
#define IMAGE_HEIGHT           240
#define JPEG_QUALITY           12        // Qualidade balanceada

// ===== CONFIGURAÇÕES DE PROCESSAMENTO =====
#define CONFIDENCE_THRESHOLD   0.7f      // Confiança mínima para envio
#define LEVEL_CHANGE_THRESHOLD 5.0f      // Mudança mínima % para trigger
#define ALERT_HIGH_LEVEL       80.0f     // Nível alto para alerta
#define ALERT_LOW_LEVEL        10.0f     // Nível baixo para alerta

// ===== CONFIGURAÇÕES DE COMUNICAÇÃO =====
#define SEND_IMAGE_ON_ALERT    true      // Enviar imagem quando houver alerta
#define MAX_RETRY_ATTEMPTS     3         // Tentativas de reenvio MQTT

#endif // CONFIG_H 
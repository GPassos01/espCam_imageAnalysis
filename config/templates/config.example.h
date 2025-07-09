/**
 * ESP32-CAM Flood Monitor - Configuration Template
 * 
 * Copie este arquivo para main/config.h e ajuste as configurações
 * para seu ambiente específico.
 * 
 * @author Gabriel Passos de Oliveira
 * @institution IGCE/UNESP - Rio Claro
 * @project ESP32-CAM Flood Monitor
 */

#ifndef CONFIG_H
#define CONFIG_H

// ================================
// 🌐 CONFIGURAÇÕES WiFi
// ================================

/**
 * IMPORTANTE: Use rede 2.4GHz (ESP32 não funciona em 5GHz)
 * Teste primeiro com hotspot do celular se tiver problemas
 */
#define WIFI_SSID        "SUA_REDE_WIFI_2.4GHZ"
#define WIFI_PASSWORD    "SUA_SENHA_WIFI"

// Configurações avançadas WiFi (opcional)
#define WIFI_MAXIMUM_RETRY     5
#define WIFI_RECONNECT_TIMEOUT 30000  // 30 segundos

// ================================
// 📡 CONFIGURAÇÕES MQTT
// ================================

/**
 * Broker MQTT - Configure conforme seu ambiente:
 * - Local: "mqtt://192.168.1.100:1883"
 * - Cloud: "mqtts://seu-broker.com:8883" (com TLS)
 */
#define MQTT_BROKER_URI  "mqtt://192.168.1.100:1883"

// Autenticação MQTT (deixe vazio "" se não usar)
#define MQTT_USERNAME    ""
#define MQTT_PASSWORD    ""

// Configurações avançadas MQTT
#define MQTT_KEEPALIVE          60
#define MQTT_QOS               1
#define MQTT_RECONNECT_DELAY   5000   // 5 segundos

// ================================
// 🔧 CONFIGURAÇÕES DO DISPOSITIVO
// ================================

/**
 * Identificação única do dispositivo
 * Use formato: ESP32CAM_XXX onde XXX é número sequencial
 */
#define DEVICE_ID        "ESP32CAM_001"
#define LOCATION_NAME    "Rio_Principal_Sensor01"

// Coordenadas GPS (opcional) - Rio Claro como exemplo
#define GPS_LATITUDE     -22.4186
#define GPS_LONGITUDE    -47.5647

// ================================
// 📷 CONFIGURAÇÕES DA CÂMERA
// ================================

/**
 * Configurações otimizadas para 8MB PSRAM
 * Testado e validado para melhor qualidade vs performance
 */

// Resolução da imagem
#define CAMERA_FRAMESIZE    FRAMESIZE_HVGA  // 480x320 (recomendado)
// Outras opções:
// FRAMESIZE_QVGA   // 320x240 (mais rápido)
// FRAMESIZE_VGA    // 640x480 (mais qualidade)

// Qualidade JPEG (0-63, menor = melhor qualidade)
#define CAMERA_JPEG_QUALITY     5    // Premium quality

// Configurações avançadas da câmera
#define CAMERA_BRIGHTNESS       0    // -2 a 2
#define CAMERA_CONTRAST         0    // -2 a 2
#define CAMERA_SATURATION       0    // -2 a 2

// ================================
// 🧠 CONFIGURAÇÕES DA VERSÃO
// ================================

/**
 * Configurações específicas da versão INTELLIGENT
 * (Ignoradas na versão SIMPLE)
 */

// Análise inteligente
#define ENABLE_ADVANCED_ANALYSIS    true
#define CHANGE_THRESHOLD            3.0f     // % mínima para considerar mudança
#define ALERT_THRESHOLD             12.0f    // % para alertas críticos
#define NOISE_FILTER_ENABLED        true

// Buffer histórico (apenas versão INTELLIGENT)
#define HISTORY_BUFFER_SIZE         3        // Número de imagens de referência
#define REFERENCE_UPDATE_INTERVAL   300000   // 5 minutos em ms

// ================================
// ⏱️ CONFIGURAÇÕES DE TIMING
// ================================

// Intervalo entre capturas
#define CAPTURE_INTERVAL_MS         15000    // 15 segundos (padrão)
// Para ambientes dinâmicos: 10000 (10s)
// Para ambientes estáticos: 30000 (30s)

// Intervalo de status/estatísticas
#define STATUS_INTERVAL_MS          300000   // 5 minutos

// Timeout para operações de rede
#define NETWORK_TIMEOUT_MS          10000    // 10 segundos

// ================================
// 🎯 CONFIGURAÇÕES CIENTÍFICAS
// ================================

/**
 * Configurações para coleta de dados científicos
 * Ajuste conforme protocolo experimental
 */

// Metadados científicos
#define EXPERIMENT_ID               "EXP_2025_001"
#define RESEARCHER_NAME             "Gabriel Passos"
#define INSTITUTION                 "IGCE/UNESP"

// Configurações de logging científico
#define ENABLE_DETAILED_LOGGING     true
#define LOG_MEMORY_USAGE           true
#define LOG_PERFORMANCE_METRICS    true

// ================================
// 🔧 CONFIGURAÇÕES AVANÇADAS
// ================================

/**
 * Configurações para usuários avançados
 * Modifique apenas se souber o que está fazendo
 */

// Sistema anti-esverdeado
#define GREEN_DETECTION_ENABLED     true
#define GREEN_CORRECTION_RETRIES    3
#define WARMUP_CAPTURES            2

// Monitoramento de sistema
#define ENABLE_WIFI_SNIFFER        false     // Pode impactar performance
#define MEMORY_MONITORING          true
#define WATCHDOG_TIMEOUT_SECONDS   30

// Otimizações de performance
#define USE_PSRAM_FOR_BUFFERS      true
#define ENABLE_CPU_BOOST           false     // Aumenta consumo
#define PARALLEL_PROCESSING        false     // Experimental

// ================================
// 🚨 CONFIGURAÇÕES DE SEGURANÇA
// ================================

/**
 * Configurações de segurança para ambiente de produção
 */

// Habilitação de TLS (apenas para produção)
#define MQTT_TLS_ENABLED           false

// Certificados TLS (apenas se TLS_ENABLED = true)
#ifdef MQTT_TLS_ENABLED
#define MQTT_CA_CERT               "-----BEGIN CERTIFICATE-----\n" \
                                   "SEU_CERTIFICADO_CA_AQUI\n" \
                                   "-----END CERTIFICATE-----"

#define MQTT_CLIENT_CERT           "-----BEGIN CERTIFICATE-----\n" \
                                   "SEU_CERTIFICADO_CLIENTE_AQUI\n" \
                                   "-----END CERTIFICATE-----"

#define MQTT_CLIENT_KEY            "-----BEGIN PRIVATE KEY-----\n" \
                                   "SUA_CHAVE_PRIVADA_AQUI\n" \
                                   "-----END PRIVATE KEY-----"
#endif

// ================================
// 📋 TÓPICOS MQTT
// ================================

/**
 * Estrutura de tópicos MQTT
 * Padrão: flood_monitor/devices/{device_id}/{data_type}
 */

// Tópicos base
#define MQTT_TOPIC_BASE            "flood_monitor/devices/" DEVICE_ID

// Tópicos específicos (gerados automaticamente)
#define MQTT_TOPIC_DATA            MQTT_TOPIC_BASE "/data"
#define MQTT_TOPIC_IMAGES          MQTT_TOPIC_BASE "/images"
#define MQTT_TOPIC_STATUS          MQTT_TOPIC_BASE "/status"
#define MQTT_TOPIC_ALERTS          MQTT_TOPIC_BASE "/alerts"
#define MQTT_TOPIC_LOGS            MQTT_TOPIC_BASE "/logs"

// ================================
// 🧪 CONFIGURAÇÕES DE DEBUG
// ================================

/**
 * Configurações para desenvolvimento e debug
 * Desabilite em produção para melhor performance
 */

// Níveis de debug
#define DEBUG_ENABLED              true
#define DEBUG_LEVEL               2        // 0=Error, 1=Warn, 2=Info, 3=Debug
#define DEBUG_MQTT_MESSAGES       false    // Log de todas mensagens MQTT
#define DEBUG_CAMERA_DETAILS      false    // Detalhes da câmera
#define DEBUG_MEMORY_TRACKING     true     // Monitoramento de memória

// Output de debug
#define DEBUG_SERIAL_ENABLED      true
#define DEBUG_MQTT_ENABLED        false    // Enviar debug via MQTT

// ================================
// ⚡ CONFIGURAÇÕES DE ENERGIA
// ================================

/**
 * Configurações para otimização de energia
 * Útil para operação com bateria
 */

// Modo de economia de energia
#define POWER_SAVE_MODE           false    // Experimental
#define DEEP_SLEEP_ENABLED        false    // Para operação com bateria
#define DEEP_SLEEP_DURATION       3600     // 1 hora em segundos

// Monitoramento de bateria (se conectada)
#define BATTERY_MONITORING        false
#define BATTERY_PIN              35        // Pino ADC para leitura

// ================================
// 📦 VALIDAÇÃO DE CONFIGURAÇÃO
// ================================

/**
 * Validações automáticas (NÃO MODIFICAR)
 */

// Validar configurações obrigatórias
#ifndef WIFI_SSID
    #error "WIFI_SSID deve ser definido!"
#endif

#ifndef MQTT_BROKER_URI
    #error "MQTT_BROKER_URI deve ser definido!"
#endif

#ifndef DEVICE_ID
    #error "DEVICE_ID deve ser definido!"
#endif

// Validar intervalos
#if CAPTURE_INTERVAL_MS < 5000
    #warning "CAPTURE_INTERVAL_MS muito baixo, pode causar problemas!"
#endif

#if CHANGE_THRESHOLD > 50.0f
    #warning "CHANGE_THRESHOLD muito alto, pode reduzir sensibilidade!"
#endif

// ================================
// 📝 INFORMAÇÕES DE BUILD
// ================================

/**
 * Informações automáticas de build
 */
#define BUILD_TIMESTAMP            __DATE__ " " __TIME__
#define FIRMWARE_VERSION           "1.0.0"
#define CONFIG_VERSION             "2025.01"

#endif // CONFIG_H

/**
 * =================================
 * 📖 GUIA DE CONFIGURAÇÃO RÁPIDA
 * =================================
 * 
 * 1. PRIMEIRO USO:
 *    - Altere WIFI_SSID e WIFI_PASSWORD
 *    - Configure MQTT_BROKER_URI 
 *    - Defina DEVICE_ID único
 * 
 * 2. VERSÃO SIMPLE:
 *    - Use configurações padrão
 *    - echo "SIMPLE" > main/ACTIVE_VERSION.txt
 * 
 * 3. VERSÃO INTELLIGENT:
 *    - Ajuste CHANGE_THRESHOLD conforme ambiente
 *    - echo "INTELLIGENT" > main/ACTIVE_VERSION.txt
 *    - Configure ENABLE_ADVANCED_ANALYSIS = true
 * 
 * 4. PRODUÇÃO:
 *    - DEBUG_ENABLED = false
 *    - MQTT_TLS_ENABLED = true (configure certificados)
 *    - Ajuste CAPTURE_INTERVAL_MS conforme necessário
 * 
 * 5. TROUBLESHOOTING:
 *    - DEBUG_ENABLED = true
 *    - DEBUG_LEVEL = 3
 *    - DEBUG_MQTT_MESSAGES = true
 * 
 * Para mais detalhes, consulte: docs/configuration.md
 */ 
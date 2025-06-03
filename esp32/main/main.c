/*
 * Projeto de Monitoramento de Enchentes - ESP32-CAM
 * Desenvolvido para Iniciação Científica - Gabriel Passos de Oliveira
 * IGCE/UNESP - 2025
 * 
 * Sistema de monitoramento inteligente com câmera OV2640
 * Análise de imagens em tempo real para detecção de enchentes
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/sys.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include "mqtt_client.h"
#include "driver/gpio.h"
#include "esp_random.h"
#include "esp_spiffs.h"

// Incluir biblioteca da câmera ESP32-CAM
#include "esp_camera.h"

// Configurações de rede
#define WIFI_SSID        "Steps 2.4G"
#define WIFI_PASS        "h%8Ka4D&"
#define WIFI_MAXIMUM_RETRY  5
#define MQTT_BROKER_URI  "mqtt://192.168.1.2:1883"
#define MQTT_USERNAME    ""
#define MQTT_PASSWORD    ""

// Tópicos MQTT
#define TOPIC_IMAGE_DATA    "enchentes/imagem/dados"
#define TOPIC_SENSOR_DATA   "enchentes/sensores"
#define TOPIC_NETWORK_STATS "enchentes/rede/estatisticas"
#define TOPIC_ALERT         "enchentes/alertas"

// Configurações do sistema - ESP32-CAM
#define IMAGE_CAPTURE_INTERVAL  30000   // 30 segundos (mais espaçado para processamento real)
#define NETWORK_MONITOR_INTERVAL 5000   // 5 segundos
#define CHANGE_THRESHOLD        0.15    // 15% de diferença (ajustado para câmera real)
#define MAX_IMAGE_SIZE          (50 * 1024) // 50KB máximo para JPEG comprimido (reduzido de 100KB)
#define MIN_IMAGE_SIZE          (2 * 1024)  // 2KB mínimo (reduzido de 5KB)
#define IMAGE_WIDTH            320     // Largura da imagem
#define IMAGE_HEIGHT           240     // Altura da imagem
#define ALERT_THRESHOLD         0.50    // 50% para alerta de enchente

// Configurações específicas da ESP32-CAM (AI-Thinker)
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

// LED Flash da ESP32-CAM
#define CAM_PIN_FLASH   4

static const char *TAG = "ENCHENTES_CAM";

// Event groups para sincronização
static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

// Handles globais
static esp_mqtt_client_handle_t mqtt_client;

// Estrutura para dados de imagem da câmera
typedef struct {
    uint8_t *buf;
    size_t len;
    uint32_t timestamp;
    size_t width;
    size_t height;
    pixformat_t format;
} camera_frame_t;

static camera_frame_t *last_frame = NULL;
static SemaphoreHandle_t camera_mutex;

// Estatísticas de rede
typedef struct {
    uint32_t bytes_enviados;
    uint32_t bytes_recebidos;
    uint32_t pacotes_enviados;
    uint32_t pacotes_recebidos;
    uint32_t imagens_enviadas;
    uint32_t imagens_descartadas;
    uint32_t capturas_falharam;
    float taxa_compressao;
} network_stats_t;

static network_stats_t net_stats = {0};

// Função para enviar imagem em chunks via MQTT
static size_t send_image_chunks(camera_frame_t* frame, const char* image_type, uint32_t image_pair_id) {
    const size_t chunk_size = 1024; // 1KB por chunk
    size_t chunks_sent = 0;
    char topic_base[80]; // Buffer para a parte base do tópico

    // Pré-formatar a parte base do tópico
    snprintf(topic_base, sizeof(topic_base), "%s/%s/%lu", TOPIC_IMAGE_DATA, image_type, image_pair_id);
    
    for (size_t offset = 0; offset < frame->len; offset += chunk_size) {
        size_t current_chunk = (offset + chunk_size > frame->len) ? 
                              (frame->len - offset) : chunk_size;
        
        char topic_full[128]; // Buffer para o tópico completo
        // Usar a base pré-formatada e adicionar o restante
        snprintf(topic_full, sizeof(topic_full), "%s/%zu/%zu", 
                topic_base, offset, frame->len);
        
        int msg_id = esp_mqtt_client_publish(mqtt_client, topic_full,
                                           (char*)(frame->buf + offset), current_chunk, 1, 0);
        if (msg_id >= 0) {
            net_stats.bytes_enviados += current_chunk;
            chunks_sent++;
        }
        
        vTaskDelay(pdMS_TO_TICKS(50)); // Delay menor para transmissão mais rápida
    }
    return chunks_sent;
}

// Configuração da câmera
static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,
    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,
    .xclk_freq_hz = 20000000,           // 20MHz
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,     // Formato JPEG para compressão automática
    .frame_size = FRAMESIZE_QVGA,       // 320x240 pixels
    .jpeg_quality = 10,                 // Melhor qualidade JPEG (10 vs 12)
    .fb_count = 2,                      // 2 buffers para melhor performance
    .fb_location = CAMERA_FB_IN_PSRAM,  // FORÇAR uso da PSRAM
    .grab_mode = CAMERA_GRAB_LATEST     // Pegar sempre a imagem mais recente
};

// Função para inicializar a câmera
static esp_err_t init_camera(void) {
    ESP_LOGI(TAG, "🎥 Inicializando câmera ESP32-CAM...");
    
    // Verificar PSRAM disponível
    size_t psram_size = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGI(TAG, "💾 PSRAM disponível: %zu bytes", psram_size);
    
    // Configurar LED flash como saída
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << CAM_PIN_FLASH),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    gpio_set_level(CAM_PIN_FLASH, 0); // LED desligado inicialmente
    
    // Primeira tentativa: PSRAM com configuração otimizada
    ESP_LOGI(TAG, "🔄 Tentativa 1: PSRAM com configuração otimizada");
    camera_config.fb_location = CAMERA_FB_IN_PSRAM;
    camera_config.jpeg_quality = 10;
    camera_config.fb_count = 2;
    
    esp_err_t err = esp_camera_init(&camera_config);
    
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "⚠️ Falha na PSRAM, tentando DRAM com 1 buffer...");
        
        // Segunda tentativa: DRAM com configuração mínima
        camera_config.fb_location = CAMERA_FB_IN_DRAM;
        camera_config.jpeg_quality = 15;
        camera_config.fb_count = 1;
        
        ESP_LOGI(TAG, "🔄 Tentativa 2: DRAM com configuração mínima");
        err = esp_camera_init(&camera_config);
        
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "❌ Falha ao inicializar câmera após 2 tentativas: %s", esp_err_to_name(err));
            ESP_LOGE(TAG, "💡 Possíveis causas:");
            ESP_LOGE(TAG, "   - Câmera não conectada corretamente");
            ESP_LOGE(TAG, "   - Alimentação insuficiente (use fonte externa 5V)");
            ESP_LOGE(TAG, "   - Memória insuficiente");
            ESP_LOGE(TAG, "   - Hardware defeituoso");
            return err;
        }
    }
    
    // Obter sensor da câmera para configurações adicionais
    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL) {
        ESP_LOGE(TAG, "❌ Falha ao obter sensor da câmera");
        return ESP_FAIL;
    }
    
    // Configurações otimizadas para detecção de enchentes
    s->set_brightness(s, 0);     // Brilho normal
    s->set_contrast(s, 2);       // Contraste aumentado para melhor detecção
    s->set_saturation(s, 0);     // Saturação normal
    s->set_special_effect(s, 0); // Sem efeitos especiais
    s->set_whitebal(s, 1);       // White balance automático
    s->set_awb_gain(s, 1);       // AWB gain automático
    s->set_wb_mode(s, 0);        // Modo white balance automático
    s->set_exposure_ctrl(s, 1);  // Controle de exposição automático
    s->set_aec2(s, 0);           // AEC sensor
    s->set_ae_level(s, 0);       // Nível de exposição automática
    s->set_aec_value(s, 300);    // Valor AEC
    s->set_gain_ctrl(s, 1);      // Controle de ganho automático
    s->set_agc_gain(s, 0);       // Ganho AGC
    s->set_gainceiling(s, (gainceiling_t)0); // Ceiling de ganho
    s->set_bpc(s, 0);            // Black pixel correction
    s->set_wpc(s, 1);            // White pixel correction
    s->set_raw_gma(s, 1);        // Gamma RAW
    s->set_lenc(s, 1);           // Lens correction
    s->set_hmirror(s, 0);        // Espelho horizontal
    s->set_vflip(s, 0);          // Flip vertical
    s->set_dcw(s, 1);            // DCW (downsize)
    s->set_colorbar(s, 0);       // Sem barra de cores
    
    ESP_LOGI(TAG, "✅ Câmera inicializada com sucesso!");
    ESP_LOGI(TAG, "📷 Configuração final: %dx%d JPEG, qualidade=%d, buffers=%d, local=%s", 
             IMAGE_WIDTH, IMAGE_HEIGHT, camera_config.jpeg_quality, 
             camera_config.fb_count,
             camera_config.fb_location == CAMERA_FB_IN_PSRAM ? "PSRAM" : "DRAM");
             
    return ESP_OK;
}

// Função para capturar imagem da câmera
static camera_frame_t* capture_camera_image(void) {
    if (xSemaphoreTake(camera_mutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
        ESP_LOGE(TAG, "❌ Timeout ao obter mutex da câmera");
        return NULL;
    }
    
    // Ativar LED flash brevemente para melhor iluminação
    gpio_set_level(CAM_PIN_FLASH, 1);
    vTaskDelay(pdMS_TO_TICKS(100)); // Flash por 100ms
    
    // Capturar frame
    camera_fb_t *fb = esp_camera_fb_get();
    
    // Desligar LED flash
    gpio_set_level(CAM_PIN_FLASH, 0);
    
    if (!fb) {
        ESP_LOGE(TAG, "❌ Falha na captura da câmera");
        net_stats.capturas_falharam++;
        xSemaphoreGive(camera_mutex);
        return NULL;
    }
    
    // Verificar se a imagem tem tamanho válido
    if (fb->len < MIN_IMAGE_SIZE || fb->len > MAX_IMAGE_SIZE) {
        ESP_LOGW(TAG, "⚠️  Tamanho de imagem inválido: %zu bytes", fb->len);
        esp_camera_fb_return(fb);
        xSemaphoreGive(camera_mutex);
        return NULL;
    }
    
    // Criar estrutura para a imagem
    camera_frame_t *frame = malloc(sizeof(camera_frame_t));
    if (!frame) {
        ESP_LOGE(TAG, "❌ Falha ao alocar memória para frame");
        esp_camera_fb_return(fb);
        xSemaphoreGive(camera_mutex);
        return NULL;
    }
    
    // Copiar dados da imagem
    frame->buf = malloc(fb->len);
    if (!frame->buf) {
        ESP_LOGE(TAG, "❌ Falha ao alocar memória para buffer de imagem");
        free(frame);
        esp_camera_fb_return(fb);
        xSemaphoreGive(camera_mutex);
        return NULL;
    }
    
    memcpy(frame->buf, fb->buf, fb->len);
    frame->len = fb->len;
    frame->width = fb->width;
    frame->height = fb->height;
    frame->format = fb->format;
    frame->timestamp = esp_timer_get_time() / 1000000LL;
    
    // Retornar frame buffer da câmera
    esp_camera_fb_return(fb);
    xSemaphoreGive(camera_mutex);
    
    ESP_LOGI(TAG, "📸 Imagem capturada: %zu bytes, %zux%zu, formato=%d", 
             frame->len, frame->width, frame->height, frame->format);
             
    return frame;
}

// Função para calcular diferença entre imagens JPEG (simplificada)
static float calculate_image_difference(camera_frame_t *img1, camera_frame_t *img2) {
    if (!img1 || !img2 || !img1->buf || !img2->buf) {
        return 1.0f; // Máxima diferença se uma imagem for inválida
    }
    
    // Para imagens JPEG, comparamos o tamanho e alguns bytes-chave
    // Esta é uma aproximação - idealmente decodificaríamos o JPEG
    
    // Se tamanhos são muito diferentes, considerar mudança significativa
    float size_ratio = (float)img1->len / img2->len;
    if (size_ratio < 0.5 || size_ratio > 2.0) {
        ESP_LOGI(TAG, "📏 Diferença de tamanho significativa: %.2f", size_ratio);
        return 0.9f; // Alta diferença por mudança de tamanho
    }
    
    // Comparar amostras de bytes em diferentes posições da imagem JPEG
    size_t sample_points = 20; // Número de pontos de amostragem
    size_t min_len = (img1->len < img2->len) ? img1->len : img2->len;
    
    if (min_len < sample_points * 10) {
        // Se imagem for muito pequena, usar comparação simples de tamanho
        return fabs(size_ratio - 1.0f);
    }
    
    uint64_t diff_sum = 0;
    for (size_t i = 0; i < sample_points; i++) {
        // Amostrar pontos distribuídos pela imagem
        size_t pos1 = (i * min_len) / sample_points;
        size_t pos2 = pos1 + (min_len / (sample_points * 2));
        
        if (pos2 < min_len) {
            // Comparar diferença absoluta nos pontos de amostra
            int diff = abs((int)img1->buf[pos1] - (int)img2->buf[pos1]);
            diff += abs((int)img1->buf[pos2] - (int)img2->buf[pos2]);
            diff_sum += diff;
        }
    }
    
    // Normalizar diferença (0-1)
    float avg_diff = (float)diff_sum / (sample_points * 2 * 255);
    
    // Combinar diferença de conteúdo com diferença de tamanho
    float total_diff = (avg_diff * 0.7) + (fabs(size_ratio - 1.0f) * 0.3);
    
    ESP_LOGI(TAG, "🔍 Análise: tamanho_ratio=%.2f, diff_conteudo=%.3f, diff_total=%.3f", 
             size_ratio, avg_diff, total_diff);
    
    return total_diff > 1.0f ? 1.0f : total_diff;
}

// Event handler para WiFi
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "Tentando reconectar ao WiFi...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "IP obtido:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// Event handler para MQTT
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Conectado");
            // Subscrever a tópicos de controle se necessário
            esp_mqtt_client_subscribe(client, "enchentes/controle/+", 0);
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Desconectado");
            break;
            
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT Subscrito, msg_id=%d", event->msg_id);
            break;
            
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT Publicado, msg_id=%d", event->msg_id);
            net_stats.pacotes_enviados++;
            break;
            
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT Dados recebidos: %.*s", event->data_len, event->data);
            // Processar comandos recebidos se necessário
            break;
            
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT Erro");
            break;
            
        default:
            break;
    }
}

// Inicializar WiFi
static void wifi_init_sta(void) {
    wifi_event_group = xEventGroupCreate();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "Inicialização WiFi finalizada.");
    
    // Aguardar conexão
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                          WIFI_CONNECTED_BIT,
                                          pdFALSE,
                                          pdFALSE,
                                          portMAX_DELAY);
    
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Conectado ao WiFi SSID:%s", WIFI_SSID);
    }
}

// Inicializar MQTT
static void mqtt_init(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.username = MQTT_USERNAME,
        .credentials.authentication.password = MQTT_PASSWORD,
    };
    
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

// Função para inicializar SPIFFS
static esp_err_t init_spiffs(void) {
    ESP_LOGI(TAG, "📂 Inicializando SPIFFS...");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "❌ Falha ao montar ou formatar o sistema de arquivos");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "❌ Partição SPIFFS não encontrada");
        } else {
            ESP_LOGE(TAG, "❌ Falha ao inicializar SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "⚠️  Falha ao obter informações do SPIFFS (%s)", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "✅ SPIFFS montado com sucesso");
    ESP_LOGI(TAG, "📊 Total: %d bytes, Usado: %d bytes", total, used);

    return ESP_OK;
}

// Task principal de captura e análise de imagens
static void camera_capture_task(void *pvParameters) {
    ESP_LOGI(TAG, "🔄 Iniciando captura de imagens com ESP32-CAM...");
    
    while (1) {
        // Aguardar conexão WiFi
        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        
        // Capturar imagem da câmera
        camera_frame_t *current_frame = capture_camera_image();
        
        if (!current_frame) {
            ESP_LOGE(TAG, "❌ Falha ao capturar imagem da câmera");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }
        
        ESP_LOGI(TAG, "📸 Imagem capturada: %zu bytes (%zux%zu)", 
                 current_frame->len, current_frame->width, current_frame->height);
        
        bool should_send = true;
        float difference = 0.0;
        
        // Comparar com última imagem se existir
        if (last_frame) {
            difference = calculate_image_difference(current_frame, last_frame);
            ESP_LOGI(TAG, "📊 Diferença calculada: %.2f%%", difference * 100);
            
            if (difference < CHANGE_THRESHOLD) {
                should_send = false;
                net_stats.imagens_descartadas++;
                ESP_LOGI(TAG, "🚫 Imagem descartada - mudança insuficiente (%.1f%% < %.1f%%)", 
                        difference * 100, CHANGE_THRESHOLD * 100);
            }
        } else {
            // Primeira imagem sempre é enviada
            ESP_LOGI(TAG, "📤 Primeira imagem - enviando automaticamente");
        }
        
        if (should_send) {
            // Para JPEG, os dados já estão comprimidos pela câmera
            size_t compressed_size = current_frame->len;
            
            // Calcular taxa de compressão estimada (JPEG vs raw)
            size_t raw_size_estimate = current_frame->width * current_frame->height * 3; // RGB estimado
            net_stats.taxa_compressao = (float)compressed_size / raw_size_estimate;
            
            // Gerar ID único para este par de imagens
            uint32_t image_pair_id = esp_timer_get_time() / 1000000LL; // timestamp como ID
            
            // Preparar dados do sensor para envio
            char sensor_data[400];
            snprintf(sensor_data, sizeof(sensor_data),
                "{\"timestamp\":%lu,\"image_pair_id\":%lu,\"current_size\":%zu,\"previous_size\":%zu,\"difference\":%.3f,\"width\":%zu,\"height\":%zu,\"format\":%d,\"location\":\"rio_monitoring_esp32cam\",\"modo\":\"camera_real\"}",
                current_frame->timestamp, image_pair_id, current_frame->len, 
                last_frame ? last_frame->len : 0, difference,
                current_frame->width, current_frame->height, current_frame->format);
            
            // Enviar dados do sensor
            int msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_SENSOR_DATA, 
                                               sensor_data, 0, 1, 0);
            if (msg_id >= 0) {
                ESP_LOGI(TAG, "📤 Dados do sensor enviados, msg_id=%d", msg_id);
                net_stats.bytes_enviados += strlen(sensor_data);
                net_stats.pacotes_enviados++;
            }
            
            size_t total_chunks = 0;
            
            // Se há diferença significativa, enviar AMBAS as imagens
            if (difference > CHANGE_THRESHOLD && last_frame) {
                ESP_LOGI(TAG, "📸 Enviando PAR DE IMAGENS para análise (diferença: %.1f%%)", difference * 100);
                
                // Enviar imagem anterior
                ESP_LOGI(TAG, "📤 Enviando imagem ANTERIOR (%zu bytes)...", last_frame->len);
                total_chunks += send_image_chunks(last_frame, "anterior", image_pair_id);
                
                // Enviar imagem atual  
                ESP_LOGI(TAG, "📤 Enviando imagem ATUAL (%zu bytes)...", current_frame->len);
                total_chunks += send_image_chunks(current_frame, "atual", image_pair_id);
                
                ESP_LOGI(TAG, "✅ Par de imagens enviado: %zu chunks totais", total_chunks);
                
            } else {
                // Primeira imagem ou diferença pequena - enviar apenas atual
                ESP_LOGI(TAG, "📤 Enviando imagem ATUAL (%zu bytes)...", current_frame->len);
                total_chunks = send_image_chunks(current_frame, "atual", image_pair_id);
            }
            
            net_stats.imagens_enviadas++;
            net_stats.pacotes_enviados += total_chunks;
            
            ESP_LOGI(TAG, "✅ Transmissão concluída: %zu chunks (compressão: %.1f%%)", 
                    total_chunks, net_stats.taxa_compressao * 100);
                
            // Verificar se é uma mudança significativa (possível enchente)
            if (difference > ALERT_THRESHOLD) { // 50% de mudança - possível alerta
                char alert_msg[350];
                snprintf(alert_msg, sizeof(alert_msg),
                    "{\"alert\":\"significant_change\",\"difference\":%.3f,\"timestamp\":%lu,\"image_pair_id\":%lu,\"current_size\":%zu,\"previous_size\":%zu,\"location\":\"rio_monitoring_esp32cam\",\"modo\":\"camera_real\"}",
                    difference, current_frame->timestamp, image_pair_id, current_frame->len, 
                    last_frame ? last_frame->len : 0);
                
                msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_ALERT, alert_msg, 0, 1, 0);
                if (msg_id >= 0) {
                    net_stats.bytes_enviados += strlen(alert_msg);
                    net_stats.pacotes_enviados++;
                }
                
                ESP_LOGW(TAG, "🚨 ALERTA: Mudança significativa detectada (%.1f%%)", difference * 100);
                
                // Piscar LED flash para indicar alerta
                for (int i = 0; i < 3; i++) {
                    gpio_set_level(CAM_PIN_FLASH, 1);
                    vTaskDelay(pdMS_TO_TICKS(200));
                    gpio_set_level(CAM_PIN_FLASH, 0);
                    vTaskDelay(pdMS_TO_TICKS(200));
                }
            }
        }
        
        // Liberar frame anterior e armazenar atual
        if (last_frame) {
            if (last_frame->buf) free(last_frame->buf);
            free(last_frame);
        }
        last_frame = current_frame;
        
        // Aguardar próxima captura
        ESP_LOGI(TAG, "⏰ Aguardando %d segundos para próxima captura...", IMAGE_CAPTURE_INTERVAL / 1000);
        vTaskDelay(pdMS_TO_TICKS(IMAGE_CAPTURE_INTERVAL));
    }
    
    vTaskDelete(NULL);
}

// Task para monitoramento de rede e sistema
static void network_monitor_task(void *pvParameters) {
    ESP_LOGI(TAG, "📡 Iniciando monitoramento de rede e sistema...");
    
    while (1) {
        // Aguardar conexão WiFi
        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        
        // Preparar relatório de estatísticas
        char stats_report[700];
        snprintf(stats_report, sizeof(stats_report),
            "{"
            "\"timestamp\":%lld,"
            "\"bytes_enviados\":%lu,"
            "\"bytes_recebidos\":%lu,"
            "\"pacotes_enviados\":%lu,"
            "\"pacotes_recebidos\":%lu,"
            "\"imagens_enviadas\":%lu,"
            "\"imagens_descartadas\":%lu,"
            "\"capturas_falharam\":%lu,"
            "\"taxa_compressao\":%.3f,"
            "\"memoria_livre\":%" PRIu32 ","
            "\"memoria_psram\":%zu,"
            "\"uptime\":%lld,"
            "\"camera_modelo\":\"ESP32-CAM\","
            "\"sensor\":\"OV2640\","
            "\"resolucao\":\"%dx%d\","
            "\"formato\":\"JPEG\","
            "\"modo\":\"camera_real\""
            "}",
            esp_timer_get_time() / 1000000LL,
            net_stats.bytes_enviados,
            net_stats.bytes_recebidos,
            net_stats.pacotes_enviados,
            net_stats.pacotes_recebidos,
            net_stats.imagens_enviadas,
            net_stats.imagens_descartadas,
            net_stats.capturas_falharam,
            net_stats.taxa_compressao,
            esp_get_free_heap_size(),
            heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
            esp_timer_get_time() / 1000000LL,
            IMAGE_WIDTH, IMAGE_HEIGHT
        );
        
        // Enviar estatísticas
        int msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_NETWORK_STATS, 
                                           stats_report, 0, 1, 0);
        if (msg_id >= 0) {
            ESP_LOGI(TAG, "📊 Estatísticas enviadas");
            net_stats.bytes_enviados += strlen(stats_report);
            net_stats.pacotes_enviados++;
        }
        
        // Calcular e exibir eficiência
        uint32_t total_imgs = net_stats.imagens_enviadas + net_stats.imagens_descartadas;
        uint32_t total_capturas = total_imgs + net_stats.capturas_falharam;
        float eficiencia = total_imgs > 0 ? (float)net_stats.imagens_descartadas / total_imgs * 100 : 0;
        float taxa_sucesso = total_capturas > 0 ? (float)(total_imgs) / total_capturas * 100 : 0;
        
        ESP_LOGI(TAG, "📈 Estatísticas do Sistema:");
        ESP_LOGI(TAG, "   📤 Dados enviados: %lu bytes em %lu pacotes", 
                net_stats.bytes_enviados, net_stats.pacotes_enviados);
        ESP_LOGI(TAG, "   📸 Imagens: %lu enviadas, %lu descartadas, %lu falhas", 
                net_stats.imagens_enviadas, net_stats.imagens_descartadas, net_stats.capturas_falharam);
        ESP_LOGI(TAG, "   📊 Eficiência: %.1f%%, Taxa sucesso: %.1f%%, Compressão: %.1f%%", 
                eficiencia, taxa_sucesso, net_stats.taxa_compressao * 100);
        ESP_LOGI(TAG, "   💾 Memória: %" PRIu32 " bytes livres, PSRAM: %zu bytes", 
                esp_get_free_heap_size(), heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        
        vTaskDelay(pdMS_TO_TICKS(NETWORK_MONITOR_INTERVAL));
    }
    
    vTaskDelete(NULL);
}

void app_main(void) {
    ESP_LOGI(TAG, "=== Iniciando Sistema de Monitoramento de Enchentes ESP32-CAM ===");
    ESP_LOGI(TAG, "🎓 Projeto IC - Gabriel Passos de Oliveira - IGCE/UNESP 2025");
    ESP_LOGI(TAG, "📷 Modo: Câmera real OV2640 - Análise de imagens em tempo real");
    
    // Inicializar NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Inicializar SPIFFS
    ESP_ERROR_CHECK(init_spiffs());
    
    // Criar mutex para câmera
    camera_mutex = xSemaphoreCreateMutex();
    if (camera_mutex == NULL) {
        ESP_LOGE(TAG, "❌ Falha ao criar mutex da câmera");
        return;
    }
    
    // Inicializar câmera
    esp_err_t camera_err = init_camera();
    if (camera_err != ESP_OK) {
        ESP_LOGE(TAG, "❌ Falha crítica na inicialização da câmera!");
        ESP_LOGE(TAG, "💡 O sistema será executado em modo de debug sem câmera");
        ESP_LOGE(TAG, "🔧 Verifique as conexões de hardware e reinicie");
        // Não fazer return, continuar com o sistema para debug
    } else {
        // Teste inicial da câmera apenas se foi inicializada com sucesso
        ESP_LOGI(TAG, "🧪 Realizando teste inicial da câmera...");
        camera_frame_t *test_frame = capture_camera_image();
        if (test_frame) {
            ESP_LOGI(TAG, "✅ Teste da câmera OK: %zu bytes", test_frame->len);
            free(test_frame->buf);
            free(test_frame);
        } else {
            ESP_LOGW(TAG, "⚠️ Teste da câmera falhou, mas continuando...");
        }
    }
    
    // Inicializar rede
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    // Inicializar WiFi
    wifi_init_sta();
    
    // Inicializar MQTT
    mqtt_init();
    
    // Aguardar estabilização das conexões
    ESP_LOGI(TAG, "⏳ Aguardando estabilização das conexões...");
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // Criar tasks
    xTaskCreate(camera_capture_task, "camera_capture", 12288, NULL, 5, NULL);
    xTaskCreate(network_monitor_task, "network_monitor", 6144, NULL, 3, NULL);
    
    ESP_LOGI(TAG, "✅ Sistema ESP32-CAM inicializado com sucesso!");
    ESP_LOGI(TAG, "📸 Captura de imagens a cada %d segundos", IMAGE_CAPTURE_INTERVAL / 1000);
    ESP_LOGI(TAG, "🔍 Threshold de mudança: %.1f%%", CHANGE_THRESHOLD * 100);
    ESP_LOGI(TAG, "📡 Monitoramento de sistema a cada %d segundos", NETWORK_MONITOR_INTERVAL / 1000);
    ESP_LOGI(TAG, "🎯 Resolução: %dx%d JPEG", IMAGE_WIDTH, IMAGE_HEIGHT);
}

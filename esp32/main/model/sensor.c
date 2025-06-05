#include "sensor.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SENSOR";

static gpio_num_t trig_pin = GPIO_NUM_NC;
static gpio_num_t echo_pin = GPIO_NUM_NC;

esp_err_t hc_sr04_init(gpio_num_t trig, gpio_num_t echo) {
    ESP_LOGI(TAG, "Inicializando HC-SR04: TRIG=%d, ECHO=%d", trig, echo);
    
    trig_pin = trig;
    echo_pin = echo;
    
    gpio_config_t trig_config = {
        .pin_bit_mask = (1ULL << trig_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    gpio_config_t echo_config = {
        .pin_bit_mask = (1ULL << echo_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    esp_err_t ret = gpio_config(&trig_config);
    if (ret != ESP_OK) return ret;
    
    ret = gpio_config(&echo_config);
    if (ret != ESP_OK) return ret;
    
    gpio_set_level(trig_pin, 0);
    ESP_LOGI(TAG, "HC-SR04 inicializado com sucesso");
    return ESP_OK;
}

float hc_sr04_read_distance(void) {
    if (trig_pin == GPIO_NUM_NC || echo_pin == GPIO_NUM_NC) {
        ESP_LOGE(TAG, "HC-SR04 não inicializado");
        return -1.0f;
    }
    
    // Trigger pulse (10µs)
    gpio_set_level(trig_pin, 1);
    esp_rom_delay_us(10);
    gpio_set_level(trig_pin, 0);
    
    // Aguardar início do echo
    uint32_t timeout = 30000; // 30ms timeout
    uint32_t start_wait = esp_timer_get_time();
    while (gpio_get_level(echo_pin) == 0) {
        if ((esp_timer_get_time() - start_wait) > timeout) {
            ESP_LOGW(TAG, "Timeout aguardando início do echo");
            return -1.0f;
        }
    }
    
    // Medir duração do echo
    uint32_t echo_start = esp_timer_get_time();
    while (gpio_get_level(echo_pin) == 1) {
        if ((esp_timer_get_time() - echo_start) > timeout) {
            ESP_LOGW(TAG, "Timeout na medição do echo");
            return -1.0f;
        }
    }
    uint32_t echo_end = esp_timer_get_time();
    
    // Calcular distância (velocidade do som = 34300 cm/s)
    uint32_t duration_us = echo_end - echo_start;
    float distance_cm = (duration_us / 2.0f) * 0.0343f;
    
    // Validar leitura (HC-SR04 range: 2-400cm)
    if (distance_cm < 2.0f || distance_cm > 400.0f) {
        ESP_LOGW(TAG, "Leitura fora do range: %.2f cm", distance_cm);
        return -1.0f;
    }
    
    return distance_cm;
}

float hc_sr04_calculate_water_level(float distance_cm, float tank_height_cm) {
    if (distance_cm < 0 || tank_height_cm <= 0) {
        return -1.0f;
    }
    
    float water_level = tank_height_cm - distance_cm;
    if (water_level < 0) water_level = 0;
    if (water_level > tank_height_cm) water_level = tank_height_cm;
    
    return (water_level / tank_height_cm) * 100.0f; // Retorna percentual
} 
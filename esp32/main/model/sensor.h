#ifndef SENSOR_H
#define SENSOR_H
#include "esp_err.h"
#include "driver/gpio.h"

esp_err_t hc_sr04_init(gpio_num_t trig_pin, gpio_num_t echo_pin);
float hc_sr04_read_distance(void);
float hc_sr04_calculate_water_level(float distance_cm, float tank_height_cm);

#endif // SENSOR_H 
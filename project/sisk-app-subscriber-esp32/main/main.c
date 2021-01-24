#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h" 

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi_app.h"
#include "mqtt_app.h"
#include "action.h"

#define WIFI_SSID               "<insert WiFi AP SSID>"
#define WIFI_PASSWD             "<insert WiFi AP password>"

#define MQTT_BROKER_ENDPOINT    "mqtt://<insert MQTT host>:<insert MQTT PORT>"  // Only TCP

static void _init_nvs(void)
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void app_main(void)
{
    _init_nvs();
    printf("SiSK project - ESP32 MQTT Client #2!\n");

    action_init();
    wifi_app_init_station(WIFI_SSID, WIFI_PASSWD);
    mqtt_app_start(MQTT_BROKER_ENDPOINT);

    while (1)
    {
        action_status();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

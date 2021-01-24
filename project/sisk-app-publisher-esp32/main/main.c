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

#include "time.h"

#include "wifi_app.h"
#include "mqtt_app.h"
#include "action.h"

#include "ds18b20.h"

#define DEVICE_SN               "DEADCAFE"

#define WIFI_SSID               "ADD"
#define WIFI_PASSWD             "ADD"

#define MQTT_BROKER_ENDPOINT    "mqtt://rpi.mateuszkapala.eu:1883"

#define MQTT_TOPIC_SEND         "sisk/temperature/room"
#define MQTT_TOPIC_DEBUG        "sisk/temperature/debug"
#define MQTT_TOPIC_SEND_DELAY   5000

#define DS18B20_GPIO            22

#define TEMPERATURE_MAX         100.0f
#define TEMPERATURE_MIN         -60.0f

static float last_good_digital_temperature_value;

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

static float get_and_handle_temperature(void)
{
    float digital_temperature = ds18b20_get_temp();

    //if temperature is too big or too low, something bad happened. Send last good_known value.
    if (digital_temperature > TEMPERATURE_MAX || digital_temperature < TEMPERATURE_MIN)
        digital_temperature = last_good_digital_temperature_value;
    else
        last_good_digital_temperature_value = digital_temperature;

    return digital_temperature;
}

void app_main(void)
{
    _init_nvs();
    printf("SiSK project - ESP32 MQTT Client #2!\n");

    action_init();                                          //not necessary
    ds18b20_init(DS18B20_GPIO);
    wifi_app_init_station(WIFI_SSID, WIFI_PASSWD);
    mqtt_app_start(MQTT_BROKER_ENDPOINT);


    while (1)
    {
        action_status();

        vTaskDelay(MQTT_TOPIC_SEND_DELAY / portTICK_PERIOD_MS);

        float digital_temperature = get_and_handle_temperature();
        char sending_message[100] = { 0 };
        // unsigned long current_time_ms = gettimeofday();

        struct timeval tv_now;
        gettimeofday(&tv_now, NULL);
        int64_t time_ms = ((int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec) / 1000;
        sprintf(sending_message, "[{\"bt\":%lld,\"n\":\"urn:dev:"DEVICE_SN":temperature\",\"v\":%f,\"u\":\"Cel\"}]", time_ms, digital_temperature);
        mqtt_publish(MQTT_TOPIC_SEND, sending_message, 1, 0);
    }
}

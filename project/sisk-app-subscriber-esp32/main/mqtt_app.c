#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"

#include "mqtt_app.h"
#include "action.h"

#define MQTT_RETRY_MAX          5
#define MQTT_TAG                "MQTT CLIENT"

#define MQTT_TOPIC_SUBSCRIBE    "sisk/temperature/room"
#define MQTT_TOPIC_DEBUG        "sisk/temperature/debug"

#define MQTT_ACCEPT_DEVICE_ID   "DEADCAFE"
#define MQTT_OWN_DEVICE_ID      "DEADBEEF"

#define SENML_TAG               "SENML PARSER"

struct mqtt_app_context
{
    float temperature;
    float timestamp;
};

static struct mqtt_app_context ctx;

static void _encode_senml_report(const enum action_result action, char *senml, size_t max_len)
{
    snprintf(senml, max_len, "[{\"bt\":%f,\"n\":\"urn:dev:%s:action\",\"vs\":\"%s\"}]", 
        ctx.timestamp, MQTT_OWN_DEVICE_ID, action_get_text_result(action));
}

static bool _decode_senml_temp(const char *senml)
{
    char device_id[9] = {0};
    float temperature = 0.0f;
    float timestamp = 0.0f;

    int32_t ret = sscanf(senml, "[{\"bt\":%f,\"n\":\"urn:dev:%8s:temperature\",\"v\":%f,\"u\":\"Cel\"}]", &timestamp, device_id, &temperature);

    if (ret < 3)
    {
        ESP_LOGE(SENML_TAG, "Failed to parse, args: %d!", ret);
        return false;
    }

    if (strcmp(device_id, MQTT_ACCEPT_DEVICE_ID) != 0)
    {
        ESP_LOGE(SENML_TAG, "Bad Device ID!");
        return false;
    }

    ctx.timestamp = timestamp;
    ctx.temperature = temperature;
    ESP_LOGI(SENML_TAG, "Acquired data -> Timestamp: %f / Temperature: %f!", ctx.timestamp, ctx.temperature);
    return true;
}

static esp_err_t _mqtt_event_handler_cb(esp_mqtt_event_t *event)
{
    esp_mqtt_client_handle_t client = event->client;
    int32_t msg_id = 0;
    char buff[256] = {0};
    char topic[256] = {0};
    bool result = true;

    switch (event->event_id) 
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, MQTT_TOPIC_SUBSCRIBE, 0);
            ESP_LOGI(MQTT_TAG, "%s(): sent subscribe successful, msg_id=%d", __func__, msg_id);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, MQTT_TOPIC_DEBUG, "subscribed", 0, 0, 0);
            ESP_LOGI(MQTT_TAG, "sent publish successful, msg_id=%d", msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DATA");
            printf("TOPIC -> %.*s\r\n", event->topic_len, event->topic);
            printf("DATA -> %.*s\r\n", event->data_len, event->data);

            snprintf(topic, sizeof(topic), "%.*s", event->topic_len, event->topic);

            if (strcmp(topic, MQTT_TOPIC_SUBSCRIBE) == 0)
            {
                result &= (snprintf(buff, sizeof(buff), "%.*s", event->data_len, event->data) < sizeof(buff));
                result &= _decode_senml_temp(buff);

                if (result)
                {
                    enum action_result action_code = action_do_something((void *)&ctx.temperature);
                    _encode_senml_report(action_code, buff, sizeof(buff));
                    msg_id = esp_mqtt_client_publish(client, MQTT_TOPIC_DEBUG, buff, 0, 0, 0);
                }                
            }
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_ERROR");
            ESP_LOGE(MQTT_TAG, "Something is not yes.");
            break;

        default:
            ESP_LOGI(MQTT_TAG, "Other event id:%d", event->event_id);
            break;
    }

    return ESP_OK;
}

static void _mqtt_app_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) 
{
    ESP_LOGD(MQTT_TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    _mqtt_event_handler_cb((esp_mqtt_event_t*)event_data);
}

void mqtt_app_start(const char* broker_endpoint)
{
    esp_mqtt_client_config_t mqtt_cfg = 
    {
        .uri = broker_endpoint,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, _mqtt_app_event_handler, client);
    esp_mqtt_client_start(client);
}
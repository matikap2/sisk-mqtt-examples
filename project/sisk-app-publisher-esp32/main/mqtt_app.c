#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

struct ctx
{
    esp_mqtt_client_handle_t client;
};

static struct ctx ctx;

static esp_err_t _mqtt_event_handler_cb(esp_mqtt_event_t *event)
{
    esp_mqtt_client_handle_t client = event->client;
    int32_t msg_id = 0;

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
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            msg_id = esp_mqtt_client_publish(client, MQTT_TOPIC_DEBUG, "recv", 0, 0, 0);
            action_do_something();
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_ERROR");
            ESP_LOGI(MQTT_TAG, "Something is not yes.");
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

    ctx.client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(ctx.client, ESP_EVENT_ANY_ID, _mqtt_app_event_handler, ctx.client);
    esp_mqtt_client_start(ctx.client);
}

void mqtt_publish(const char *topic, const char *message, const int qos, const bool retain)
{
    int len = strlen(message);
    esp_mqtt_client_publish(ctx.client, topic, message, len, qos, retain);
}
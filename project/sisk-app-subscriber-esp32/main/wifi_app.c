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

#include "wifi_app.h"

#define WIFI_RETRY_MAX          5
#define WIFI_TAG                "WIFI STATION"

#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT           BIT1

struct wifi_app_context
{
    uint8_t wifi_retry_num;
    EventGroupHandle_t wifi_event_group;
};

static struct wifi_app_context ctx;

static void _wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        ESP_LOGI(WIFI_TAG, "Wi-Fi connection start!");
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        ESP_LOGI(WIFI_TAG, "Wi-Fi connected!");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        ESP_LOGI(WIFI_TAG, "Wi-Fi disconnected!");

        if (ctx.wifi_retry_num < WIFI_RETRY_MAX) 
        {
            esp_wifi_connect();
            ctx.wifi_retry_num ++;
            ESP_LOGI(WIFI_TAG, "Retry to connect to the AP...");
        } 
        else 
        {
            ESP_LOGI(WIFI_TAG, "Failed to connect to AP!");
            xEventGroupSetBits(ctx.wifi_event_group, WIFI_FAIL_BIT);
        }
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;

        ESP_LOGI(WIFI_TAG, "Got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        ctx.wifi_retry_num = 0;
        xEventGroupSetBits(ctx.wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_app_init_station(const char *ssid, const char *passwd)
{
    ctx.wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                                ESP_EVENT_ANY_ID,
                                                &_wifi_event_handler,
                                                NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                                IP_EVENT_STA_GOT_IP,
                                                &_wifi_event_handler,
                                                NULL));

    wifi_config_t wifi_config = 
    {
        .sta = 
        {
            .ssid = {0},
            .password = {0},

            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	        .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    strncpy((char*)wifi_config.sta.ssid, ssid, strlen(ssid));
    strncpy((char*)wifi_config.sta.password, passwd, strlen(passwd));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(WIFI_TAG, "%s() finished!", __func__);

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(ctx.wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) 
    {
        ESP_LOGI(WIFI_TAG, "Connected to AP -> SSID:%s password:%s",
                 ssid, passwd);
    } 
    else if (bits & WIFI_FAIL_BIT) 
    {
        ESP_LOGI(WIFI_TAG, "Failed to connect to AP -> SSID:%s, password:%s",
                 ssid, passwd);
    } 
    else 
    {
        ESP_LOGE(WIFI_TAG, "Something is not yes.");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &_wifi_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &_wifi_event_handler ));
    vEventGroupDelete(ctx.wifi_event_group);
}

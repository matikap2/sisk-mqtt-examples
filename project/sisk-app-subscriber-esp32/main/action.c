#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "action.h"

#define GPIO_LED        4

#define ACTION_TAG      "ACTION"

#define LOW_TEMP        22.0f
#define HIGH_TEMP       24.0f
 
static const char *actions[ACTION_TOP] =
{
    [ACTION_NO_CHANGE]  = "ACTION_NO_CHANGE",
    [ACTION_HEATER_ON]  = "ACTION_HEATER_ON",
    [ACTION_HEATER_OFF] = "ACTION_HEATER_OFF",
};

struct action_context
{
    bool enable;
};

static struct action_context ctx;

void _change_heater_state(bool state)
{
    gpio_set_level(GPIO_LED, state);
}

void action_init(void)
{
    gpio_config_t gpio =
    {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << GPIO_LED),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };

    gpio_config(&gpio);

    ctx.enable = false;
}

enum action_result action_do_something(void *param)
{
    float *temp = (float *)param;
    enum action_result action_code = ACTION_TOP;

    if (ctx.enable && *temp > HIGH_TEMP)
    {
        ctx.enable = false;
        action_code = ACTION_HEATER_OFF;
    }
    else if (!ctx.enable && *temp < LOW_TEMP)
    {
        ctx.enable = true;
        action_code = ACTION_HEATER_ON;
    }
    else
    {
        action_code = ACTION_NO_CHANGE;
    }

    ESP_LOGW(ACTION_TAG, "%s -> Enable: %s / Temperature: %f", action_get_text_result(action_code), ctx.enable ? "true" : "false", *temp);
    _change_heater_state(ctx.enable);
    return action_code;
}

void action_status(void)
{
    ESP_LOGI(ACTION_TAG, "Current status: %d", ctx.enable);
}

const char* action_get_text_result(enum action_result action)
{
    if (action >= ACTION_TOP)
        return NULL;
    else
        return actions[action];
}
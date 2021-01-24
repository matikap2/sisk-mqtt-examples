#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"


#include "action.h"

#define GPIO_LED        2

#define ACTION_TAG      "ACTION"

struct action_context
{
    bool gpio_enable;
};

static struct action_context ctx;

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

    ctx.gpio_enable = false;
}

void action_do_something(void)
{
    ctx.gpio_enable ^= true;
    gpio_set_level(GPIO_LED, ctx.gpio_enable);

}

void action_status(void)
{
    ESP_LOGI(ACTION_TAG, "Current status: %d", ctx.gpio_enable);
}
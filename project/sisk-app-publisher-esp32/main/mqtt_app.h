#ifndef _MQTT_APP_H_
#define _MQTT_APP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void mqtt_app_start(const char *broker_endpoint);

void mqtt_publish(const char *topic, const char *message, const int qos, const bool retain);

#ifdef __cplusplus
}
#endif

#endif /* _WIFI_A_MQTT_APP_H_PP_H_ */
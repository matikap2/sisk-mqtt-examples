#ifndef _MQTT_APP_H_
#define _MQTT_APP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void mqtt_app_start(const char *broker_endpoint);

#ifdef __cplusplus
}
#endif

#endif /* _MQTT_APP_H_ */
#ifndef _WIFI_APP_H_
#define _WIFI_APP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void wifi_app_init_station(const char *ssid, const char *passwd);

#ifdef __cplusplus
}
#endif

#endif /* _WIFI_APP_H_ */
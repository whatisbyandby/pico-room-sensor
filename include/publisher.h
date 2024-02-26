
#pragma once
#include "lwip/apps/mqtt.h"

typedef struct {
    mqtt_client_t *client;

} publisher_t;

typedef enum {
    PUBLISHER_OK,
    PUBLISHER_ERROR_NO_CLIENT,
    PUBLISHER_ERROR_CONNECT,
    PUBLISHER_ERROR_PUBLISH,
} publisher_error_t;

publisher_error_t publisher_init(publisher_t *publisher);
publisher_error_t publish(publisher_t *publisher, const char *topic, const char *payload);

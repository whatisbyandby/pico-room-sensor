
#include "publisher.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "secrets.h"



void publish_callback(void *args, err_t err)
{
    printf("publish_callback\n");
    if (err != ERR_OK)
    {
        printf("Publish error: %d\n", err);
    }

}

void connection_callback(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    printf("connection_callback\n");
    if (status == MQTT_CONNECT_ACCEPTED)
    {
        printf("mqtt connected\n");
    }
    else
    {
        printf("mqtt connection failed\n");
    }
}

publisher_error_t publisher_init(publisher_t *publisher)
{
    mqtt_client_t *client = mqtt_client_new();
    if (client == NULL)
    {
        return PUBLISHER_ERROR_NO_CLIENT;
    }

    publisher->client = client;

    const struct mqtt_connect_client_info_t client_info = {
        .client_id = "pico",
        .client_user = NULL,
        .client_pass = NULL,
        .will_topic = NULL,
        .will_msg = NULL,
        .will_qos = 0,
        .will_retain = 0,
        .keep_alive = 60,
    };

    ip_addr_t ipaddr;
    const char *ip_str = "192.168.1.88";

    if (ipaddr_aton(ip_str, &ipaddr))
    {
        // Conversion succeeded
        printf("IP address: %s converted successfully.\n", ip_str);
        // Here you can use 'ipaddr' with other LwIP functions
    }
    else
    {
        // Conversion failed
        printf("Failed to convert IP address: %s\n", ip_str);
    }

    cyw43_arch_lwip_begin();
    err_t err = mqtt_client_connect(client, &ipaddr, MQTT_SERVER_PORT, &connection_callback, 0, &client_info);
    cyw43_arch_lwip_end();

    if (err != ERR_OK)
    {
        return PUBLISHER_ERROR_CONNECT;
    }

    return PUBLISHER_OK;
}

publisher_error_t publish(publisher_t *publisher, const char *topic, const char *payload)
{
    cyw43_arch_lwip_begin();
    err_t err = mqtt_publish(publisher->client, topic, payload, strlen(payload), 0, 0, &publish_callback, 0);
    cyw43_arch_lwip_end();

    if (err != ERR_OK)
    {
        return PUBLISHER_ERROR_PUBLISH;
    }

    return PUBLISHER_OK;
}

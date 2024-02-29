#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "secrets.h"
#include "hardware/watchdog.h"
#include "room_sensor.h"
#include "lwip/apps/mqtt.h"
#include "pico/cyw43_arch.h"

#include <cJSON.h>


void reset()
{
	// Set the watchdog timer to a short interval
	watchdog_enable(1, 1); // 1ms timeout, 1 for panic on timeout
	while (1)
		tight_loop_contents();
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
		printf("status: %d\n", status);
	}
}

bool mqtt_client_init(mqtt_client_t *client, char *device_id)
{
	if (client == NULL)
	{
		printf("mqtt_client_new failed\n");
		return false;
	}

	char will_topic_buffer[100];
	sprintf(will_topic_buffer, "device/%s/status", device_id);

	const struct mqtt_connect_client_info_t client_info = {
		.client_id = device_id,
		.client_user = MQTT_USER,
		.client_pass = MQTT_PASS,
		.will_topic = will_topic_buffer,
		.will_msg = "offline",
		.will_qos = 1,
		.will_retain = 1,
		.keep_alive = 60,
	};

	ip_addr_t ipaddr;
	const char *ip_str = MQTT_SERVER_HOST;

	if (!ipaddr_aton(ip_str, &ipaddr))
	{
		// Conversion failed
		printf("Failed to convert IP address: %s\n", ip_str);
		return false;
	}

	cyw43_arch_lwip_begin();
	err_t err = mqtt_client_connect(client, &ipaddr, MQTT_SERVER_PORT, connection_callback, 0, &client_info);
	cyw43_arch_lwip_end();

	if (err != ERR_OK)
	{
		printf("mqtt_client_connect failed: %d\n", err);
		return false;
	}
}

bool initalize_wifi()
{

	if (cyw43_arch_init())
	{
		printf("failed to initialise\n");
		return false;
	}

	cyw43_arch_enable_sta_mode();
}

bool connect_wifi()
{
	if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000))
	{
		printf("failed to connect\n");
		return false;
	}

	return true;
}

static bool is_configured = false;
static bool handle_config = true;

void subscribe_filter(void *arg, const char *topic, u32_t tot_len)
{

	printf("subscribe_filter\n");
	printf("topic: %s\n", topic);

	if (strcmp(topic, "device/E661385283731631/config")) {
		printf("Sending along to config handler");
	}
	else {
		printf("Not sending along to config handler");
	}
}

void subscribe_callback(void *arg, const u8_t *data, u16_t len, u8_t flags)
{

	if (!handle_config) return;
	printf("data: %s", data);
	
	room_sensor_t *sensor = (room_sensor_t *)arg;
	cJSON *json = cJSON_Parse(data);

	const cJSON *i2c_config = NULL;

	sensor_error_t err = handle_configuration_message(sensor, json);

	if (err != SENSOR_OK)
	{
		printf("Failed to configure sensor, resetting\n");
		reset();
	}

	sensor->is_configured = true;
}

int main()
{

	stdio_init_all();

	pico_unique_board_id_t id;

	int id_len = 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1;

	char id_buffer[id_len];
	pico_get_unique_board_id_string(id_buffer, id_len);

	printf("unique id: %s\n", id_buffer);

	room_sensor_t sensor;
	sensor.is_configured = false;

	initalize_wifi();
	connect_wifi();

	mqtt_client_t *client = mqtt_client_new();
	mqtt_client_init(client, id_buffer);

	mqtt_set_inpub_callback(client, subscribe_filter, subscribe_callback, &sensor);

	sensor.id = id_buffer;
	sensor.client = client;

	send_base_message(&sensor);

	char topic[100];
	sprintf(topic, "device/%s/config", id_buffer);
	printf("Subscribing\n");
	mqtt_subscribe(client, topic, 0, NULL, NULL);
	printf("Subscribed\n");

	while (!sensor.is_configured)
	{
		printf("Waiting for configuration\n");
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(250);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(250);
		sleep_ms(500);
	}


	si7021_t si7021 = {
		.i2c = sensor.i2c
	};

	si7021_init(&si7021);

	
	sensor.si7021 = &si7021;

	send_discovery_message(&sensor);

	while (true)
	{

		room_sensor_loop(&sensor);
		sleep_ms(10000);
	}
}

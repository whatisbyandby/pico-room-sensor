#include <stdio.h>
#include <string.h>
#include "lwip/apps/mqtt.h"
#include "pico/unique_id.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "cJSON.h"

#include "room_sensor.h"

float convert_celsius_to_fahrenheit(float celsius)
{
    return (celsius * 9 / 5) + 32;
}

sensor_error_t publish(mqtt_client_t *client, const char *topic, const char *message)
{
    if (client == NULL)
    {
        return SENSOR_ERROR_NO_CLIENT;
    }

    if (mqtt_client_is_connected(client) == false)
    {
        
    }

    mqtt_publish(client, topic, message, strlen(message), 0, 0, NULL, NULL);
    return SENSOR_OK;
}

sensor_error_t room_sensor_init(room_sensor_t *sensor)
{

    if (sensor->si7021 == NULL)
    {
        return SENSOR_ERROR_NO_SENSOR;
    }

    if (sensor->client == NULL)
    {
        return SENSOR_ERROR_NO_CLIENT;
    }

    return SENSOR_OK;
}

sensor_error_t send_base_message(room_sensor_t *sensor)
{
    char base_topic[100];
    sprintf(base_topic, "device/%s/status", sensor->id);
    publish(sensor->client, base_topic, "online");
}

sensor_error_t send_discovery_message(room_sensor_t *sensor)
{   

    char availability_topic_buffer[100];
	sprintf(availability_topic_buffer, "device/%s/status", sensor->id);

    char temperature_entity[100];
    sprintf(temperature_entity, "homeassistant/sensor/%s_temperature/config", sensor->id);
    cJSON *temp_discovery_message = cJSON_CreateObject();
    char temperature_name[100];
    sprintf(temperature_name, "%s_temperature", sensor->room);
    cJSON_AddStringToObject(temp_discovery_message, "name", temperature_name);
    char temp_state_topic_buffer[100];
    sprintf(temp_state_topic_buffer, "home/%s/temperature", sensor->room);
    cJSON_AddStringToObject(temp_discovery_message, "state_topic", temp_state_topic_buffer);
    cJSON_AddStringToObject(temp_discovery_message, "unit_of_measurement", "°F");
    cJSON_AddStringToObject(temp_discovery_message, "device_class", "temperature");
    // cJSON_AddStringToObject(temp_discovery_message, "availability_topic", availability_topic_buffer);
    char temp_unique_id_buffer[100];
    sprintf(temp_unique_id_buffer, "%s_temperature", sensor->id);
    cJSON_AddStringToObject(temp_discovery_message, "unique_id", temp_unique_id_buffer);
    char *temperature_message_string = cJSON_Print(temp_discovery_message);
    publish(sensor->client, temperature_entity, temperature_message_string);


    char humidity_entity[100];
    sprintf(humidity_entity, "homeassistant/sensor/%s_humidity/config", sensor->id);

    cJSON *humid_discovery_message = cJSON_CreateObject();
    char device_name[100];
    sprintf(device_name, "%s_humidity", sensor->room);
    cJSON_AddStringToObject(humid_discovery_message, "name", device_name);
    char humid_state_topic_buffer[100];
    sprintf(humid_state_topic_buffer, "home/%s/humidity", sensor->room);
    cJSON_AddStringToObject(humid_discovery_message, "state_topic", humid_state_topic_buffer);
    cJSON_AddStringToObject(humid_discovery_message, "unit_of_measurement", "\%");
    cJSON_AddStringToObject(humid_discovery_message, "device_class", "humidity");
    // cJSON_AddStringToObject(humid_discovery_message, "availability_topic", availability_topic_buffer);
    char humid_unique_id_buffer[100];
    sprintf(humid_unique_id_buffer, "%s_humidity", sensor->id);
    cJSON_AddStringToObject(humid_discovery_message, "unique_id", humid_unique_id_buffer);
    char *humid_discovery_message_string = cJSON_Print(humid_discovery_message);
    publish(sensor->client, humidity_entity, humid_discovery_message_string);
}

sensor_error_t room_sensor_loop(room_sensor_t *sensor)
{
    si7021_reading_t new_reading;
    read_humidity(sensor->si7021, &new_reading);
    read_temperature(sensor->si7021, &new_reading);
    char topic_buffer[100];

    sprintf(topic_buffer, "home/%s/temperature", sensor->room);

    char temperature_buffer[25];
    sprintf(temperature_buffer, "%.1f", convert_celsius_to_fahrenheit(new_reading.temperature));
    publish(sensor->client, topic_buffer, temperature_buffer);

    sprintf(topic_buffer, "home/%s/humidity", sensor->room);

    char humidity_buffer[25];
    sprintf(humidity_buffer, "%.1f", new_reading.humidity);
    publish(sensor->client, topic_buffer, humidity_buffer);
}


void parse_i2c_configuration(cJSON *i2c_config, room_sensor_t *sensor)
{
    cJSON *sda = cJSON_GetObjectItem(i2c_config, "sda");
    cJSON *scl = cJSON_GetObjectItem(i2c_config, "scl");
    cJSON *i2c_bus = cJSON_GetObjectItem(i2c_config, "bus");

    sensor->sda = sda->valueint;
    sensor->scl = scl->valueint;
    sensor->i2c = i2c_bus->valueint == 0 ? i2c0 : i2c1;

}

void initalize_i2c(room_sensor_t *sensor)
{
    i2c_init(sensor->i2c, 100 * 1000);
    gpio_set_function(sensor->sda, GPIO_FUNC_I2C);
    gpio_set_function(sensor->scl, GPIO_FUNC_I2C);
    gpio_pull_up(sensor->sda);
    gpio_pull_up(sensor->scl);
}

sensor_error_t handle_configuration_message(room_sensor_t *sensor, cJSON *config)
{
    cJSON *temp_unit = cJSON_GetObjectItem(config, "temp_unit");
    if (temp_unit->valueint == 0)
    {
        sensor->temp_unit = CELSIUS;
    }
    else
    {
        sensor->temp_unit = FAHRENHEIT;
    }

    cJSON *room = cJSON_GetObjectItem(config, "room");
    sensor->room = room->valuestring;

    cJSON *i2c_config = cJSON_GetObjectItem(config, "i2c");

    parse_i2c_configuration(i2c_config, sensor);
    initalize_i2c(sensor);

    sensor->is_configured = true;
    return SENSOR_OK;
}







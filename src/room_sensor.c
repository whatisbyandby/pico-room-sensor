#include "room_sensor.h"

float convert_celsius_to_fahrenheit(float celsius) {
    return (celsius * 9 / 5) + 32;
}

sensor_error_t room_sensor_init(room_sensor_t *sensor){

    pico_unique_board_id_t id;
    pico_get_unique_board_id(&id);
    sensor->id = id;

    if (sensor->publisher == NULL) {
        return SENSOR_ERROR_NO_PUBLISHER;
    }

    if (sensor->si7021 == NULL) {
        return SENSOR_ERROR_NO_SENSOR;
    }

    return SENSOR_OK;
}

sensor_error_t send_discovery_message(room_sensor_t *sensor){

    //TODO: implement this function

}



sensor_error_t room_sensor_loop(room_sensor_t *sensor){

    si7021_reading_t new_reading;
    read_humidity(sensor->si7021, &new_reading);
    read_temperature(sensor->si7021, &new_reading);

    char reading_buffer[100];
    if (sensor->temp_unit == CELSIUS) {
        snprintf(reading_buffer, 100, "Temperature: %.2f, Humidity: %.2f", new_reading.temperature, new_reading.humidity);
    } else {
        float fahrenheit = convert_celsius_to_fahrenheit(new_reading.temperature);
        snprintf(reading_buffer, 100, "Temperature: %.2f, Humidity: %.2f", fahrenheit, new_reading.humidity);
    }

    publish(sensor->publisher, "room_sensor", reading_buffer);

}
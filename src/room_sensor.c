#include "room_sensor.h"
#include "cJSON.h"
#include <pb_encode.h>
#include "reading.pb.h"

float convert_celsius_to_fahrenheit(float celsius)
{
    return (celsius * 9 / 5) + 32;
}

sensor_error_t room_sensor_init(room_sensor_t *sensor)
{

    pico_unique_board_id_t id;
    pico_get_unique_board_id(&id);
    sensor->id = id;

    if (sensor->publisher == NULL)
    {
        return SENSOR_ERROR_NO_PUBLISHER;
    }

    if (sensor->si7021 == NULL)
    {
        return SENSOR_ERROR_NO_SENSOR;
    }

    return SENSOR_OK;
}

sensor_error_t send_discovery_message(room_sensor_t *sensor)
{

    // TODO: implement this function
}

sensor_error_t room_sensor_loop(room_sensor_t *sensor)
{

    si7021_reading_t new_reading;
    read_humidity(sensor->si7021, &new_reading);
    read_temperature(sensor->si7021, &new_reading);

    // char device_id[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
    // pico_get_unique_board_id_string(device_id, 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);

    // cJSON *root = cJSON_CreateObject();
    // cJSON_AddStringToObject(root, "device_id", device_id);
    // cJSON_AddStringToObject(root, "temperature_unit", sensor->temp_unit == CELSIUS ? "C" : "F");
    // cJSON_AddNumberToObject(root, "temperature", sensor->temp_unit == CELSIUS ? new_reading.temperature : convert_celsius_to_fahrenheit(new_reading.temperature));
    // cJSON_AddNumberToObject(root, "humidity", new_reading.humidity);

    // const char *payload = cJSON_Print(root);

    uint8_t buffer[128];
    size_t message_length;
    bool status;

    Reading message = Reading_init_zero;

    /* Create a stream that will write to our buffer. */
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    /* Fill in the lucky number */
    message.temperature = new_reading.temperature;
    message.humidity = new_reading.humidity;

    /* Now we are ready to encode the message! */
    status = pb_encode(&stream, Reading_fields, &message);
    message_length = stream.bytes_written;

    /* Then just check for any errors.. */
    if (!status)
    {
        printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return 1;
    }

    publish(sensor->publisher, "room_sensor", buffer);

    // cJSON_Delete(root);
}
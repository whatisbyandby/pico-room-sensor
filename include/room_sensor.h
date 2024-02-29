#include "pico/unique_id.h"
#include "si7021.h"
#include "lwip/apps/mqtt.h"
#include "hardware/i2c.h"
#include "cJSON.h"

//define the enum for temperature units
typedef enum {
    CELSIUS,
    FAHRENHEIT
} temp_unit_t;

typedef struct {
    char *id;
    temp_unit_t temp_unit;
    mqtt_client_t *client;
    si7021_t *si7021;
    bool is_configured;
    char *room;
    uint32_t interval;
    int sda;
    int scl;
    i2c_inst_t *i2c;
} room_sensor_t;


// define enum for sensor error states
typedef enum {
    SENSOR_OK,
    SENSOR_ERROR,
    SENSOR_ERROR_NO_CLIENT,
    SENSOR_ERROR_NO_SENSOR,
} sensor_error_t;

sensor_error_t handle_configuration_message(room_sensor_t *sensor, cJSON *config);

sensor_error_t room_sensor_init(room_sensor_t *sensor);

sensor_error_t send_base_message(room_sensor_t *sensor);

sensor_error_t send_discovery_message(room_sensor_t *sensor);

sensor_error_t room_sensor_loop(room_sensor_t *sensor);

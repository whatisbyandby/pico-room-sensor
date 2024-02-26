#include "pico/unique_id.h"
#include "publisher.h"
#include "si7021.h"

//define the enum for temperature units
typedef enum {
    CELSIUS,
    FAHRENHEIT
} temp_unit_t;

typedef struct {
    pico_unique_board_id_t id;
    publisher_t *publisher;
    temp_unit_t temp_unit;
    si7021_t *si7021;
} room_sensor_t;


// define enum for sensor error states
typedef enum {
    SENSOR_OK,
    SENSOR_ERROR,
    SENSOR_ERROR_NO_PUBLISHER,
    SENSOR_ERROR_NO_SENSOR,
} sensor_error_t;

sensor_error_t room_sensor_init(room_sensor_t *sensor);

sensor_error_t send_discovery_message(room_sensor_t *sensor);

sensor_error_t room_sensor_loop(room_sensor_t *sensor);
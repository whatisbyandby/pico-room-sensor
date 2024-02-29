#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "si7021.h"

#define SI7021_MEASURE_TEMP_HOLD 0xE3
#define SI7021_MEASURE_HUM_HOLD 0xE5
#define SI7021_MEASURE_TEMP_NO_HOLD 0xF3
#define SI7021_MEASURE_HUM_NO_HOLD 0xF5
#define SI7021_READ_TEMP_PREV 0xE0
#define SI7021_RESET 0xFE
#define SI7021_READ_USER_REG 0xE7
#define SI7021_WRITE_USER_REG 0xE6
#define SI7021_READ_HEATER_REG 0x11
#define SI7021_WRITE_HEATER_REG 0x51


si7021_error_t si7021_init(si7021_t *sensor) {
    if (sensor->i2c == NULL) {
        return SI7021_ERR_NO_I2C;
    }

    uint8_t read_buffer[1] = {0};
    int num_bytes = i2c_read_blocking(i2c0, 0x40, read_buffer, 1, false);

    if (num_bytes != 1) {
        return SI7021_ERR_NO_RESPONSE;
    }
    return SI7021_OK;
}


si7021_error_t write_then_read(si7021_t *sensor, uint8_t command, uint16_t *result) {
    uint8_t read_buffer[2] = {0, 0};
    int num_bytes = i2c_write_blocking(sensor->i2c, SENSOR_ADDR, &command, 1, false);

    if (num_bytes != 1) {
        return SI7021_ERR_WRITE_FAIL;
    }

    num_bytes = i2c_read_blocking(sensor->i2c, SENSOR_ADDR, read_buffer, 2, false);

    if (num_bytes != 2) {
        return SI7021_ERR_READ_FAIL;
    }

    *result = (read_buffer[0] << 8) | read_buffer[1];
    return SI7021_OK;
}


si7021_error_t read_temperature(si7021_t *sensor, si7021_reading_t *reading) {

    uint16_t temp_code;
    si7021_error_t err = write_then_read(sensor, SI7021_MEASURE_TEMP_HOLD, &temp_code);

    if (err != SI7021_OK) {
        return err;
    }

    reading->temperature = (175.72 * temp_code / 65536) - 46.85;
    return SI7021_OK;
}

si7021_error_t read_humidity(si7021_t *sensor, si7021_reading_t *reading) {

    uint16_t hum_code;
    si7021_error_t err = write_then_read(sensor, SI7021_MEASURE_HUM_HOLD, &hum_code);

    if (err != SI7021_OK) {
        return err;
    }

    // convert the humidity code to a percentage and store in the new reading
    reading->humidity = (125 * hum_code / 65536) - 6;
    return SI7021_OK;
}




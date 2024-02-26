#pragma once

#include "hardware/i2c.h"

//define the enum for the si7021 error states
typedef enum {
    SI7021_OK,
    SI7021_ERR_NO_I2C,
    SI7021_ERR_INVALID_ADDR,
    SI7021_ERR_NO_RESPONSE,
    SI7021_ERR_WRITE_FAIL,
    SI7021_ERR_READ_FAIL
} si7021_error_t;

typedef struct si7021 {
    i2c_inst_t *i2c;
    uint8_t addr;
} si7021_t;

typedef struct {
    float temperature;
    float humidity;
} si7021_reading_t;

si7021_error_t si7021_init(si7021_t *sensor);

si7021_error_t read_temperature(si7021_t *sensor, si7021_reading_t *reading);
si7021_error_t read_humidity(si7021_t *sensor, si7021_reading_t *reading);
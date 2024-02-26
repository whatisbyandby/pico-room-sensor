#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "secrets.h"
#include "wifi.h"
#include "publisher.h"
#include "hardware/watchdog.h"
#include "si7021.h"
#include "room_sensor.h"

#include "config.h"


void initalize_i2c(uint sda, uint scl, uint baudrate) {
  i2c_init(i2c0, baudrate);
  gpio_set_function(sda, GPIO_FUNC_I2C);
  gpio_set_function(scl, GPIO_FUNC_I2C);
  gpio_pull_up(sda);
  gpio_pull_up(scl);
}

void reset() {
    // Set the watchdog timer to a short interval
    watchdog_enable(1, 1); // 1ms timeout, 1 for panic on timeout
    while (1) tight_loop_contents();
}


int main() {

  stdio_init_all();
  bool wifi_initalized = wifi_init();

  if (!wifi_initalized) {
    printf("Unable to initalize the WiFi chip\n");

  }

  int num_retries = 0;
  while (!wifi_connect(WIFI_SSID, WIFI_PASSWORD)) {
    if (num_retries > 5) {
      printf("Unable to connect to WiFi after 5 retries\n");
      printf("Resetting the device\n");
      reset();
    }
    printf("Unable to connect to WiFi\n");
    printf("Trying again in 5 seconds\n");
    sleep_ms(5000);
    num_retries++;
  }

  initalize_i2c(SDA_PIN, SCL_PIN, I2C_RATE);

  publisher_t publisher;
  publisher_init(&publisher);

  si7021_t si7021 = {
    .i2c = i2c0,
    .addr = SI7021_ADDRESS
  };
  si7021_init(&si7021);


  room_sensor_t sensor = {
    .si7021 = &si7021,
    .publisher = &publisher,
    .temp_unit = FAHRENHEIT
  };

  while (true) {

    room_sensor_loop(&sensor);

    sleep_ms(1000);
  }
}
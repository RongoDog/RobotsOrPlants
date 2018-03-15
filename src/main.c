#include "temperature_sensor.h"
#include "motor_controller.h"
#include "ultra_sonic_sensor.h"
#include "flame_sensors.h"
#include <stdlib.h>
#include <pigpio.h>

typedef enum states_enum
{
    searching = 0,
    safety = 1,
    pinpointing = 2,
    redirect = 3,
    attack_flame = 4
    extinguish_flame = 5
} states;

states current_state;

int main() {
    // Begin by initializing gpio
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Failed to initialize GPIO interface");
        exit(1);
    }
    
    // The motors should be initially off, but their gpio set
    initialize_motors();
    current_state = searching;
    
    // We need threads for all our sensors
    pthread_t flame_sensor_thread;
    pthread_t temperature_sensor_thread;
    pthread_t ultra_sonic_sensor_thread;
    
    // Create the required threads
    pthread_create(&flame_sensor_thread, NULL, initialize_flame_sensors, NULL);
    pthread_create(&temperature_sensor_thread, NULL, intialize_temperature_sensor, NULL);
    pthread_create(&ultra_sonic_sensor_thread, NULL, initialize_vision, NULL);
}

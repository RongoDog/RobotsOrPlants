#include "temperature_sensor.h"
#include "motor_controller.h"
#include "ultra_sonic_sensor.h"
#include "flame_sensors.h"
#include "pinout_definitions.h"
#include <stdio.h>
#include <stdlib.h>
#include <pigpio.h>
#include <semaphore.h>
#include <sys/msg.h>

static int message_queue_id = -1; 
static int temp_data;
static int dist_data;
static int flame_data_front;
static int flame_data_back;
static int flame_data_left;
static int flame_data_right;

typedef enum states_enum
{
    searching = 0,
    safety = 1,
    pinpointing = 2,
    redirect = 3,
    attack_flame = 4,
    extinguish_flame = 5
} states;

states current_state;

int start_message_queue()
{
    message_queue_id = msgget((key_t)MESSAGE_QUEUE, 0666 | IPC_CREAT);
    if (message_queue_id == -1) return(-1);
    return(0);
}

void end_message_queue()
{
    (void)msgctl(message_queue_id, IPC_RMID, 0);
    message_queue_id = -1;
}

int receive_sensor_message(struct sensor_data *data_to_read)
{
    struct sensor_message my_msg;
    if (msgrcv(message_queue_id, (void *)&my_msg, sizeof(*data_to_read), SENSOR_MESSAGE, 0) == -1) {
        return(-1);
    }
    *data_to_read = my_msg.data;
    return(0);
}

void send_flame_control_data(direction dir)
{
    struct flame_sensor_message my_msg;
    struct direction_data data;
    data.value = dir;

    my_msg.data = data;
    my_msg.msg_key = FLAME_SENSOR_CONTROL_MESSAGE;

    msgsnd(message_queue_id, (void *)&my_msg, sizeof(data), IPC_NOWAIT);
}

int main() {
    // Begin by initializing gpio
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Failed to initialize GPIO interface");
        exit(1);
    }
    
    // The motors should be initially off, but their gpio set
    initialize_motors();
    current_state = searching;

    // We start the message queue
    if (start_message_queue()) {
        fprintf(stderr, "Failed to start message queue\n");
        exit(1);
    }

    // Initialize the semaphore for the i2c communication.
    sem_t i2c_semaphore;
    sem_init(&i2c_semaphore, 0, 1);
    
    // We need threads for all our sensors
    pthread_t flame_sensor_thread;
    pthread_t temperature_sensor_thread;
    pthread_t ultra_sonic_sensor_thread;

    // We create the thread info structure
    struct thread_info *info = malloc(sizeof(struct thread_info));
    info->semaphore = &i2c_semaphore;
    info->message_queue_id = &message_queue_id; 
    
    // Create the required threads
    pthread_create(&flame_sensor_thread, NULL, initialize_flame_sensors, (void *)info);
    pthread_create(&temperature_sensor_thread, NULL, initialize_temperature_sensor, (void *)info);
    pthread_create(&ultra_sonic_sensor_thread, NULL, initialize_vision, (void *)info);

    struct sensor_data received_data;
    while(1) {
        // Receive Message
        if (receive_sensor_message(&received_data) < 0) {
            fprintf(stderr, "Failed to receive sensor data\n");
            continue;
        }
        switch (received_data.type) {
            case temperature_data:
                temp_data = received_data.value;
            case flame_data:
                switch (received_data.dir) {
                    case front:
                        flame_data_front = received_data.value;
                    case back:
                        flame_data_back = received_data.value;
                    case left:
                        flame_data_left = received_data.value;
                    case right:
                        flame_data_right = received_data.value;
                    default: 
                        continue;
                }
            case distance_data:
                dist_data = received_data.value;
            default:
                continue;
        }

        // Interpret
    }

    void** exit_status; 
    pthread_join(flame_sensor_thread, exit_status);
    pthread_join(temperature_sensor_thread, exit_status);
    pthread_join(ultra_sonic_sensor_thread, exit_status);

    exit(0);
    return 0;
}

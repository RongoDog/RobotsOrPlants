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
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/time.h>

#define DISTANCE_THRESHOLD 40
#define FLAME_LR_SAFETY_THRESHOLD 300
#define FLAME_FRONT_SAFETY_THRESHOLD 300
#define FLAME_BACK_SAFETY_THRESHOLD 900

#define FLAME_DETECTION_THRESHOLD 1500

static int message_queue_id = -1; 
static int temp_data = 0;
static int dist_data = 0;
// Initialize to absurdly high value
static int flame_data_front = 10000;
static int flame_data_back = 10000;
static int flame_data_left = 10000;
static int flame_data_right = 10000;

static unsigned long pinpoint_start_time; 

static unsigned long extinguish_start_time;
static int extinguish_dir = 0;

typedef enum states_enum
{
    searching = 0,
    safety = 1,
    pinpointing = 2,
    redirect = 3,
    attack_flame = 4,
    extinguish_flame = 5
} states;

typedef enum pinpoint_states_enum
{
    left_pinpoint = 0,
    right_pinpoint = 1,
    back_pinpoint = 2
} pinpoint_states;

static double max_flame_found = 0;


states current_state = searching;
pinpoint_states currently_pinpointing;

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
    if (msgrcv(message_queue_id, (void *)&my_msg, sizeof(struct sensor_data), SENSOR_MESSAGE, 0) == -1) {
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

unsigned long get_current_time() {
    return (unsigned long)time(NULL);
}

void searching_state() { 
    send_flame_control_data(not_specified);
    if (dist_data < DISTANCE_THRESHOLD) {
        current_state = redirect;
    } else if (flame_data_front < FLAME_DETECTION_THRESHOLD || \
        flame_data_back < FLAME_DETECTION_THRESHOLD || \
        flame_data_left < FLAME_DETECTION_THRESHOLD || \
        flame_data_right < FLAME_DETECTION_THRESHOLD) {
        send_flame_control_data(front);
        pinpoint_start_time = get_current_time();
        printf("Pinpoint time: %d\n", pinpoint_start_time);
        if (flame_data_front < FLAME_DETECTION_THRESHOLD) {
            current_state = extinguish_flame;
            extinguish_start_time = get_current_time();
        } else if (flame_data_right < FLAME_DETECTION_THRESHOLD) {
            current_state = pinpointing;
            currently_pinpointing = right_pinpoint;
        } else if (flame_data_back < FLAME_DETECTION_THRESHOLD) {
            current_state = pinpointing;
            currently_pinpointing = back_pinpoint;
        } else if (flame_data_left < FLAME_DETECTION_THRESHOLD) {
            current_state = pinpointing;
            currently_pinpointing = left_pinpoint;
        }
    }
    return;
}

void safety_state() {
    if (flame_data_front > FLAME_FRONT_SAFETY_THRESHOLD && \
        flame_data_back > FLAME_BACK_SAFETY_THRESHOLD && \
        flame_data_left > FLAME_LR_SAFETY_THRESHOLD && \
        flame_data_right > FLAME_LR_SAFETY_THRESHOLD) {
        current_state = searching;
    }
    return;
}

void pinpointing_state() {
    if (flame_data_front < FLAME_DETECTION_THRESHOLD) {
        current_state = extinguish_flame;
        return;
    }
    if ((pinpoint_start_time + 4) < get_current_time()) {
        current_state = searching;
    }
}

void redirect_state() {
    if (dist_data > DISTANCE_THRESHOLD) {
        current_state = searching;
    }
    return;
}

void extinguish_flame_state() {
    if ((extinguish_start_time + 2) < get_current_time()) {
        current_state = searching;
    }
}

void analyze_state() {
    switch(current_state) {
        case searching:
            searching_state();
            break;
        case safety:
            safety_state();
            break;
        case pinpointing:
            pinpointing_state();
            break;
        case redirect:
            redirect_state();
            break;
        case extinguish_flame:
            extinguish_flame_state();
            break;
        default:
            current_state = searching;
    }
}

void execute_state() {
    switch(current_state) {
        case searching:
            pump_off();
            drive_forward();
            break;
        case safety:
            pump_off();
            if (flame_data_front < FLAME_FRONT_SAFETY_THRESHOLD) {
                drive_backward();
            } else {
                drive_forward();
            }
            break;
        case pinpointing:
            pump_off();
            switch(currently_pinpointing) {
                case left_pinpoint:
                    reverse_arc_left();
                    break;
                case right_pinpoint:
                    reverse_arc_right();
                    break;
                case back_pinpoint:
                    sharp_right();
                    break;
            }
            break;
        case redirect:
            pump_off();
            sharp_left();
            break;
        case extinguish_flame:
            extinguish_dir = rand() & 1;
            if (extinguish_dir) {
                sharp_left();
                extinguish_dir = 0;
            } else {
                sharp_right();
                extinguish_dir = 1;
            }
            pump_on(); 
            break;
        default:
            pump_off();
            break;
    }
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
    //pthread_t temperature_sensor_thread;
    pthread_t ultra_sonic_sensor_thread;

    // We create the thread info structure
    struct thread_info *info = malloc(sizeof(struct thread_info));
    info->semaphore = &i2c_semaphore;
    info->message_queue_id = &message_queue_id; 
    
    // Create the required threads
    pthread_create(&flame_sensor_thread, NULL, initialize_flame_sensors, (void *)info);
    //pthread_create(&temperature_sensor_thread, NULL, initialize_temperature_sensor, (void *)info);
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
                break;
            case flame_data:
                switch (received_data.dir) {
                    case front:
                        flame_data_front = received_data.value;
                        break;
                    case back:
                        flame_data_back = received_data.value;
                        break;
                    case left:
                        flame_data_left = received_data.value;
                        break;
                    case right:
                        flame_data_right = received_data.value;
                        break;
                    default: 
                        continue;
                }
                break;
            case distance_data:
                dist_data = received_data.value;
                break;
            default:
                continue;
        }
        printf("Distance: %d, Front flame: %d, Back flame: %d, Left flame: %d, Right flame: %d\n", dist_data, flame_data_front, flame_data_back, flame_data_left, flame_data_right);
        // Determine next state
        analyze_state();
        // Execute on state
        execute_state();

    }

    void** exit_status; 
    pthread_join(flame_sensor_thread, exit_status);
    //pthread_join(temperature_sensor_thread, exit_status);
    pthread_join(ultra_sonic_sensor_thread, exit_status);

    exit(0);
    return 0;
}

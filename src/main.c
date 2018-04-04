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

#define DISTANCE_THRESHOLD 40
#define FLAME_LR_SAFETY_THRESHOLD 0.8
#define FLAME_FRONT_SAFETY_THRESHOLD 0.8
#define FLAME_BACK_SAFETY_THRESHOLD 0.8

#define FLAME_ATTACK_THRESHOLD 0.9
#define FLAME_DETECTION_THRESHOLD 1.5

static int message_queue_id = -1; 
static double temp_data = 0;
static double dist_data = 0;
static double flame_data_front = 0;
static double flame_data_back = 0;
static double flame_data_left = 0;
static double flame_data_right = 0;

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
    stage_one = 0,
    stage_two = 1,
    stage_three = 2,
    stage_four = 3
} pinpoint_states;

static double max_flame_found = 0;


states current_state = searching;
pinpoint_states currently_pinpointing = stage_one;

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

void searching_state() { 
    if (dist_data < DISTANCE_THRESHOLD) {
        current_state = redirect;
    } else if (flame_data_front < FLAME_FRONT_SAFETY_THRESHOLD && \
        flame_data_back < FLAME_BACK_SAFETY_THRESHOLD && \
        flame_data_left < FLAME_LR_SAFETY_THRESHOLD && \
        flame_data_right < FLAME_LR_SAFETY_THRESHOLD) {
        current_state = safety;
    } else if (flame_data_front < FLAME_DETECTION_THRESHOLD && \
        flame_data_back < FLAME_DETECTION_THRESHOLD && \
        flame_data_left < FLAME_DETECTION_THRESHOLD && \
        flame_data_right < FLAME_DETECTION_THRESHOLD) {
        current_state = pinpointing;
        if (flame_data_front < FLAME_DETECTION_THRESHOLD) {
            currently_pinpointing = stage_two;
        } else {
            currently_pinpointing = stage_one;
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
    switch(currently_pinpointing) {
        case stage_one:
            if (flame_data_front < FLAME_DETECTION_THRESHOLD) {
                currently_pinpointing = stage_two;
            }
            break;
        case stage_two:
            if (flame_data_front < max_flame_found) {
                max_flame_found = flame_data_front;
            }
            if (flame_data_front > FLAME_DETECTION_THRESHOLD) {
                currently_pinpointing = stage_three;
            }
            break;
        case stage_three:
            if (flame_data_front < max_flame_found) {
                max_flame_found = flame_data_front;
            }
            if (flame_data_front > FLAME_DETECTION_THRESHOLD) {
                currently_pinpointing = stage_four;
            }
            break;
        case stage_four:
            if (((flame_data_front - max_flame_found)/FLAME_DETECTION_THRESHOLD) < 0.1) {
                max_flame_found = 0;
                current_state = attack_flame;
            }
            break;
        default:
            break;
    }
}

void redirect_state() {
    if (dist_data > DISTANCE_THRESHOLD) {
        current_state = searching;
    }
    return;
}

void attack_flame_state() {
    if (flame_data_front < FLAME_ATTACK_THRESHOLD) {
        current_state = extinguish_flame;
    }
}

void extinguish_flame_state() {
    if (flame_data_front > FLAME_DETECTION_THRESHOLD) {
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
        case attack_flame:
            attack_flame_state();
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
                case stage_one:
                    sharp_right();
                    break;
                case stage_two:
                    sharp_right();
                    break;
                case stage_three:
                    sharp_left();
                    break;
                case stage_four:
                    sharp_right();
                    break;
                default:
                    sharp_right();
                    break;
            }
            break;
        case redirect:
            pump_off();
            sharp_left();
            break;
        case attack_flame:
            pump_off();
            drive_forward();
            break;
        case extinguish_flame:
            motors_off();
            //pump_on(); Remove comment when required
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
    pthread_t temperature_sensor_thread;
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
        printf("Received some data %f\n", received_data.value);
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


        /*
        fprintf(stdout, "Temperature: %f, Distance: %f\n", temp_data, dist_data);
        fprintf(stdout, "Flame_Front: %f, Flame_Back: %f, Flame_Left: %f, Flame_Right: %f\n",
                flame_data_front, flame_data_back, flame_data_left, flame_data_right);
        */

        analyze_state();
        execute_state();
        //sleep(1);
        // Interpret

    }

    void** exit_status; 
    pthread_join(flame_sensor_thread, exit_status);
    pthread_join(temperature_sensor_thread, exit_status);
    pthread_join(ultra_sonic_sensor_thread, exit_status);

    exit(0);
    return 0;
}

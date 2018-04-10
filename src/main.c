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
#define FLAME_ATTACK_THRESHOLD 700

#define PINPOINT_TIMOUT 5
#define EXINGUISH_TIMOUT 3

static int message_queue_id = -1; 
static int temp_data = 0;
static int dist_data = 0;
static int redirect_direction = 0;
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
    back_pinpoint = 2,
    right_sweep = 3,
    left_sweep = 4,
    final = 5
} pinpoint_states;

static int max_flame_found = 0;


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
    // Don't hit a wall
    if (dist_data < DISTANCE_THRESHOLD) {
        current_state = redirect;
        // We want to redirect in an arbitrary direction
        redirect_direction = 1;
    // If any sensors catches a flame, we want to pinpoint it
    } else if (flame_data_front < FLAME_DETECTION_THRESHOLD || \
        flame_data_back < FLAME_DETECTION_THRESHOLD || \
        flame_data_left < FLAME_DETECTION_THRESHOLD || \
        flame_data_right < FLAME_DETECTION_THRESHOLD) {
        // To begin the pinpointing state,
        // we focus control on the front sensor and
        // initialize the max flame fount to 0
        send_flame_control_data(front);
        pinpoint_start_time = get_current_time();
        max_flame_found = 0;
        current_state = pinpointing;
        // If the front catches the flame, we skip a few states
        // in the pinpointing, otherwise, we engage the appropriate
        // initial action
        if (flame_data_front < FLAME_DETECTION_THRESHOLD) {
            currently_pinpointing = right_sweep;
        } else if (flame_data_right < FLAME_DETECTION_THRESHOLD) {
            flame_data_right = 10000;
            currently_pinpointing = right_pinpoint;
        } else if (flame_data_back < FLAME_DETECTION_THRESHOLD) {
            flame_data_back = 10000;
            currently_pinpointing = back_pinpoint;
        } else if (flame_data_left < FLAME_DETECTION_THRESHOLD) {
            flame_data_left = 10000;
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
    // Always update the max flame found
    if (max_flame_found > flame_data_front) {
        max_flame_found = flame_data_front;
    }
    switch(currently_pinpointing) {
        // We right sweep the whole detection range of the front sensor
        case right_sweep:
            if (flame_data_front > FLAME_DETECTION_THRESHOLD) {
                currently_pinpointing = left_sweep;
                return;
            } else if (flame_data_front < FLAME_ATTACK_THRESHOLD) {
                // Switch to the extinguish state
                current_state = extinguish_flame;
                extinguish_start_time = get_current_time();
                return;
            }
            break;
        // We left sweep the whole detection range of the left sensor
        case left_sweep:
            if (flame_data_front > FLAME_DETECTION_THRESHOLD) {
                current_state = searching;
                // Ensure that the flame sensor module is capturing
                // in all directions
                send_flame_control_data(not_specified);
                return;
            } else if (flame_data_front < FLAME_ATTACK_THRESHOLD) {
                // Switch to the extinguish state
                current_state = extinguish_flame;
                extinguish_start_time = get_current_time();
                return;
            }
            break;
        /*
        // At this stage the maximum flame value is likely found
        // We shoot when ready
        case final:
            // Within a 10% margin of the max, we shoot
            if (flame_data_front < FLAME_ATTACK_THRESHOLD) {
                // We need to cleanup
                max_flame_found = 0;
                // Switch to the extinguish state
                current_state = extinguish_flame;
                extinguish_start_time = get_current_time();
                return;
            }
            break;
        */
        // Represents the initial search
        default:
            if (flame_data_front < FLAME_ATTACK_THRESHOLD) {
                // Switch to the extinguish state
                current_state = extinguish_flame;
                extinguish_start_time = get_current_time();
                return;
            }
            break;
    }
    // Lost flames are ignored after 6 seconds. 
    if ((pinpoint_start_time + PINPOINT_TIMOUT) < get_current_time()) {
        current_state = searching;
        // Ensure that the flame sensor module is capturing
        // in all directions
        send_flame_control_data(not_specified);
    }
}

void redirect_state() {
    // This state purely avoid obstacles
    if (dist_data > DISTANCE_THRESHOLD) {
        current_state = searching;
    }
    return;
}

void extinguish_flame_state() {
    // The extinguish state complete when flame is no longer detected
    if ((extinguish_start_time + EXINGUISH_TIMOUT) < get_current_time() || \
        flame_data_front > FLAME_DETECTION_THRESHOLD) {
        current_state = searching;
        // Ensure that the flame sensor module is capturing
        // in all directions
        send_flame_control_data(not_specified);
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
                    reverse_arc_left(160);
                    break;
                case right_pinpoint:
                    reverse_arc_right(160);
                    break;
                case back_pinpoint:
                    sharp_right_var(160);
                    break;
                case right_sweep:
                    sharp_right_var(160);
                    break;
                case left_sweep:
                    sharp_left_var(160);
                    break;
                case final:
                    sharp_right_var(160);
                    break;
            }
            break;
        case redirect:
            pump_off();
            // Redirect depending on the state
            if (redirect_direction) {
                sharp_left();
            } else {
                sharp_right();
            }
            break;
        case extinguish_flame:
            // We want to spray slightly sporatically
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
        // We store the data based on the type
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
                        // Bad data
                        continue;
                }
                break;
            case distance_data:
                dist_data = received_data.value;
                break;
            default:
                continue;
        }
        printf("Front sensor data: %d\n", flame_data_front);
        printf("Distance data: %d\n", dist_data);
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

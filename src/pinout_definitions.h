#include <semaphore.h>

#define OUTPUT_PIN_E_RIGHT 14
#define OUTPUT_PIN_A_RIGHT 15
#define OUTPUT_PIN_B_RIGHT 18
#define OUTPUT_PIN_E_LEFT 25
#define OUTPUT_PIN_A_LEFT 8
#define OUTPUT_PIN_B_LEFT 7
#define OUTPUT_PIN_E_PUMP 10
#define OUTPUT_PIN_A_PUMP 9
#define OUTPUT_PIN_B_PUMP 11

// Known to work
#define ULTRA_SONIC_OUTPUT_PIN 4
#define ULTRA_SONIC_INPUT_PIN 17

#define MICRO_SEC_IN_SEC 1000000

#define MESSAGE_QUEUE 1234

#define SENSOR_MESSAGE 1 
#define FLAME_SENSOR_CONTROL_MESSAGE 2

struct thread_info {
	sem_t *semaphore;
	int *message_queue_id; 
};

typedef enum direction_enum
{
	front = 1,
	left = 0,
	right = 3,
	back = 2,
	not_specified = 4
} direction;

typedef enum data_type_enum
{
    temperature_data = 0,
    distance_data = 1,
    flame_data = 2
} data_type;

struct sensor_data {
	data_type type; 
	direction dir; 
	int value;
};

struct sensor_message {
	long int msg_key;
	struct sensor_data data; 
};

struct direction_data {
	direction value;
};

struct flame_sensor_message {
	long int msg_key;
	struct direction_data data; 
};
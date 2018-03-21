#include <pigpio.h>
#include <sys/types.h>
#include <sys/msg.h>
#include "pinout_definitions.h"
#include "ultra_sonic_sensor.h"

#define USECOND_CENTIMETER_RATIO 58

#define MIN_RANGE_LENGTH 2
#define MAX_RANGE_LENGTH 500

int *message_queue_id;

typedef struct time_tracker {
	long int start;
	long int end;
} time_tracker;

void send_distance_data(double distance)
{
	struct sensor_message my_msg;
	struct sensor_data data;
	data.type = distance_data;
	data.value = distance;
	data.dir = front;

	my_msg.data = data;
	my_msg.msg_key = SENSOR_MESSAGE;
	msgsnd(*message_queue_id, (void *)&my_msg, sizeof(data), IPC_NOWAIT);
}


void callback_function(int gpio, int level, unsigned int tick, void *t) {
	if (level == 1) {
		((time_tracker *)t)->start = tick;
		return;
	} else if (level == 0) {
		((time_tracker *)t)->end = tick;
		long int elapsed_time = (((time_tracker *)t)->end)-(((time_tracker *)t)->start);
		double distance = (double)elapsed_time/USECOND_CENTIMETER_RATIO;
		if (distance < MIN_RANGE_LENGTH || distance > MAX_RANGE_LENGTH) {
			// We skip this reading since it is erronous either a loop in the tick
			// counter or a hardware error. 
			return;
		}
		send_distance_data(distance);
	}
}

void *initialize_vision(void *arg) {
	struct thread_info *info = (struct thread_info *)arg;
	message_queue_id = info->message_queue_id;

	gpioSetMode(ULTRA_SONIC_OUTPUT_PIN, PI_OUTPUT); 
	gpioSetMode(ULTRA_SONIC_INPUT_PIN, PI_INPUT);

	struct time_tracker t = { .start = 0, .end = 0 };

	gpioSetAlertFuncEx(ULTRA_SONIC_INPUT_PIN, callback_function, &t);
	// This is the main while loop
	while(1) { 
		gpioTrigger(ULTRA_SONIC_OUTPUT_PIN, 10, 1);
	}
}

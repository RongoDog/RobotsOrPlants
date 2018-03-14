#include <pigpio.h>
#include <sys/types.h>
#include "pinout_definitions.h"
#include "ultra_sonic_sensor.h"

#define USECOND_CENTIMETER_RATIO 58

#define MIN_RANGE_LENGTH 2
#define MAX_RANGE_LENGTH 500

typedef struct time_tracker {
	long int start;
	long int end;
	long int current_distance;
} time_tracker;

void callback_function(int gpio, int level, unsigned int tick, void *t) {
	if (level == 1) {
		((time_tracker *)t)->start = tick;
		return;
	} else if (level == 0) {
		((time_tracker *)t)->end = tick;
		long int elapsed_time = (((time_tracker *)t)->end)-(((time_tracker *)t)->start);
		long int distance = elapsed_time/USECOND_CENTIMETER_RATIO;
		if (distance < MIN_RANGE_LENGTH || distance > MAX_RANGE_LENGTH) {
			// We skip this reading since it is erronous either a loop in the tick
			// counter or a hardware error. 
			return;
		}
		((time_tracker*)t)->current_distance = elapsed_time/USECOND_CENTIMETER_RATIO;
		// Do something with output
	}
}

void *initialize_vision(void *arg) {
	gpioSetMode(ULTRA_SONIC_OUTPUT_PIN, PI_OUTPUT); 
	gpioSetMode(ULTRA_SONIC_INPUT_PIN, PI_INPUT);

	struct time_tracker t = { .start = 0, .end = 0 };

	gpioSetAlertFuncEx(ULTRA_SONIC_INPUT_PIN, callback_function, &t);
	// This is the main while loop
	while(1) { 
		gpioTrigger(ULTRA_SONIC_OUTPUT_PIN, 10, 1);
		gpioDelay(MICRO_SEC_IN_SEC/2);
	}
}

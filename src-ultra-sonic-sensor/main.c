#include <pigpio.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>

#define OUTPUT_PIN 23
#define INPUT_PIN 24

#define MICRO_SEC_IN_SEC 1000000
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
		// We're going to want to direct this stdout to a file. 
		fprintf(stdout, "The length of the pulse is: %ld micro sec, the distance is %ld\n", elapsed_time, distance);
	}
}

int main() {
	if (gpioInitialise() < 0) {
		fprintf(stderr, "Failed to initialize GPIO interface");
		exit(1);
	}
	gpioSetMode(OUTPUT_PIN, PI_OUTPUT); 
	gpioSetMode(INPUT_PIN, PI_INPUT);

	struct time_tracker t = { .start = 0, .end = 0 };

	gpioSetAlertFuncEx(INPUT_PIN, callback_function, &t);
	//gpioSetWatchdog(INPUT_PIN, 300);

	// This is the main while loop
	while(1) { 
		gpioTrigger(OUTPUT_PIN, 10, 1);
		gpioDelay(MICRO_SEC_IN_SEC/2);
	}
}

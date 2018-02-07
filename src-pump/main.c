#include <pigpio.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>

#define OUTPUT_PIN_E 14
#define OUTPUT_PIN_A 15

#define MICRO_SEC_IN_SEC 1000000

bool initializePump(){ //returns 1 if failure to initialize
	
	if (gpioInitialise() < 0) {
		fprintf(stderr, "Failed to initialize GPIO interface");
		return 1;
	}
	gpioSetMode(OUTPUT_PIN_E, PI_OUTPUT);

	gpioWrite(OUTPUT_PIN_E, 0);
	gpioWrite(OUTPUT_PIN_A, 1);
	
	return 0;
}

void DrivePump(){
	gpioWrite(OUTPUT_PIN_E, 1);
}

void StopDrivingPump(){
	gpioWrite(OUTPUT_PIN_E, 0);
}

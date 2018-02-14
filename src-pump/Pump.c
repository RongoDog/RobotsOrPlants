#include <Pump.h>

bool initializePump(){ //returns 1 if failure to initialize
	
	if (gpioInitialise() < 0) {
		fprintf(stderr, "Failed to initialize GPIO interface");
		return 1;
	}
	gpioSetMode(OUTPUT_PIN_E, PI_OUTPUT);

	gpioWrite(OUTPUT_PIN_E, 0);
	gpioWrite(OUTPUT_PIN_B, 0);
	gpioWrite(OUTPUT_PIN_A, 1);
	
	return 0;
}

void DrivePump(){
	gpioWrite(OUTPUT_PIN_E, 1);
}

void StopDrivingPump(){
	gpioWrite(OUTPUT_PIN_E, 0);
}


#include <pigpio.h>
#include <stdlib.h>

#define OUTPUT_PIN_E_LEFT 14
#define OUTPUT_PIN_A_LEFT 15
#define OUTPUT_PIN_B_LEFT 18

#define OUTPUT_PIN_E_RIGHT 25
#define OUTPUT_PIN_A_RIGHT 8
#define OUTPUT_PIN_B_RIGHT 7

#define OUTPUT_PIN_E_PUMP 10
#define OUTPUT_PIN_A_PUMP 9
#define OUTPUT_PIN_B_PUMP 11


void drive_forward() {
	gpioWrite(OUTPUT_PIN_E_RIGHT, 1);
	gpioWrite(OUTPUT_PIN_A_RIGHT, 1);
	gpioWrite(OUTPUT_PIN_B_RIGHT, 0);

	gpioWrite(OUTPUT_PIN_E_LEFT, 1);
	gpioWrite(OUTPUT_PIN_A_LEFT, 1);
	gpioWrite(OUTPUT_PIN_B_LEFT, 0);
}

void drive_backward() {
	gpioWrite(OUTPUT_PIN_E_RIGHT, 1);
	gpioWrite(OUTPUT_PIN_A_RIGHT, 0);
	gpioWrite(OUTPUT_PIN_B_RIGHT, 1);

	gpioWrite(OUTPUT_PIN_E_LEFT, 1);
	gpioWrite(OUTPUT_PIN_A_LEFT, 0);
	gpioWrite(OUTPUT_PIN_B_LEFT, 1);
}

void arc_left() {
	gpioWrite(OUTPUT_PIN_E_RIGHT, 1);
	gpioWrite(OUTPUT_PIN_A_RIGHT, 1);
	gpioWrite(OUTPUT_PIN_B_RIGHT, 0);

	gpioWrite(OUTPUT_PIN_E_LEFT, 0);
	gpioWrite(OUTPUT_PIN_A_LEFT, 0);
	gpioWrite(OUTPUT_PIN_B_LEFT, 0);
}

void arc_right() {
	gpioWrite(OUTPUT_PIN_E_RIGHT, 0);
	gpioWrite(OUTPUT_PIN_A_RIGHT, 0);
	gpioWrite(OUTPUT_PIN_B_RIGHT, 0);

	gpioWrite(OUTPUT_PIN_E_LEFT, 1);
	gpioWrite(OUTPUT_PIN_A_LEFT, 1);
	gpioWrite(OUTPUT_PIN_B_LEFT, 0);
}

void sharp_left() {
	gpioWrite(OUTPUT_PIN_E_RIGHT, 1);
	gpioWrite(OUTPUT_PIN_A_RIGHT, 1);
	gpioWrite(OUTPUT_PIN_B_RIGHT, 0);

	gpioWrite(OUTPUT_PIN_E_LEFT, 1);
	gpioWrite(OUTPUT_PIN_A_LEFT, 0);
	gpioWrite(OUTPUT_PIN_B_LEFT, 1);
}

void sharp_right() {
	gpioWrite(OUTPUT_PIN_E_RIGHT, 1);
	gpioWrite(OUTPUT_PIN_A_RIGHT, 0);
	gpioWrite(OUTPUT_PIN_B_RIGHT, 1);

	gpioWrite(OUTPUT_PIN_E_LEFT, 1);
	gpioWrite(OUTPUT_PIN_A_LEFT, 1);
	gpioWrite(OUTPUT_PIN_B_LEFT, 0);
}

void motors_off() {
	gpioWrite(OUTPUT_PIN_E_RIGHT, 0);
	gpioWrite(OUTPUT_PIN_A_RIGHT, 0);
	gpioWrite(OUTPUT_PIN_B_RIGHT, 0);

	gpioWrite(OUTPUT_PIN_E_LEFT, 0);
	gpioWrite(OUTPUT_PIN_A_LEFT, 0);
	gpioWrite(OUTPUT_PIN_B_LEFT, 0);
}

void pump_on() {
	gpioWrite(OUTPUT_PIN_E_PUMP, 1);
	gpioWrite(OUTPUT_PIN_A_PUMP, 1);
	gpioWrite(OUTPUT_PIN_B_PUMP, 0);
}

void pump_off() {
	gpioWrite(OUTPUT_PIN_E_PUMP, 0);
	gpioWrite(OUTPUT_PIN_A_PUMP, 0);
	gpioWrite(OUTPUT_PIN_B_PUMP, 0);
}

int main() {
	if (gpioInitialise() < 0) {
		exit(1);
	}
	gpioSetMode(OUTPUT_PIN_E_PUMP, PI_OUTPUT); 
	gpioSetMode(OUTPUT_PIN_A_PUMP, PI_OUTPUT); 
	gpioSetMode(OUTPUT_PIN_B_PUMP, PI_OUTPUT); 
	gpioSetMode(OUTPUT_PIN_E_LEFT, PI_OUTPUT); 
	gpioSetMode(OUTPUT_PIN_A_LEFT, PI_OUTPUT); 
	gpioSetMode(OUTPUT_PIN_B_LEFT, PI_OUTPUT); 
	drive_backward();
	pump_off();
	while(1) {
		//pump_on();
		gpioDelay(100000);
		gpioDelay(100000);
	}
}

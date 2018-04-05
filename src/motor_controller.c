#include <pigpio.h>
#include "pinout_definitions.h"
#include "motor_controller.h"

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

void reverse_arc_left() {
	gpioWrite(OUTPUT_PIN_E_RIGHT, 1);
	gpioWrite(OUTPUT_PIN_A_RIGHT, 0);
	gpioWrite(OUTPUT_PIN_B_RIGHT, 1);

	gpioWrite(OUTPUT_PIN_E_LEFT, 0);
	gpioWrite(OUTPUT_PIN_A_LEFT, 0);
	gpioWrite(OUTPUT_PIN_B_LEFT, 0);
}

void reverse_arc_right() {
	gpioWrite(OUTPUT_PIN_E_RIGHT, 0);
	gpioWrite(OUTPUT_PIN_A_RIGHT, 0);
	gpioWrite(OUTPUT_PIN_B_RIGHT, 0);

	gpioWrite(OUTPUT_PIN_E_LEFT, 1);
	gpioWrite(OUTPUT_PIN_A_LEFT, 0);
	gpioWrite(OUTPUT_PIN_B_LEFT, 1);
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

void initialize_motors() {
	gpioSetMode(OUTPUT_PIN_E_RIGHT, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_B_RIGHT, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_A_RIGHT, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_E_LEFT, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_B_LEFT, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_A_LEFT, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_E_PUMP, PI_OUTPUT); 
	gpioSetMode(OUTPUT_PIN_A_PUMP, PI_OUTPUT); 
	gpioSetMode(OUTPUT_PIN_B_PUMP, PI_OUTPUT); 
	motors_off();
	pump_off();
}

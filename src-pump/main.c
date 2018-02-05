#include <pigpio.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>

#define OUTPUT_PIN_E_1 14
#define OUTPUT_PIN_B_1 15
#define OUTPUT_PIN_A_1 18



#define MICRO_SEC_IN_SEC 1000000

int main() {
	
	if (gpioInitialise() < 0) {
		fprintf(stderr, "Failed to initialize GPIO interface");
		exit(1);
	}
	gpioSetMode(OUTPUT_PIN_E_1, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_B_1, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_A_1, PI_OUTPUT);
	gpioWrite(OUTPUT_PIN_E_1, 1);
	gpioWrite(OUTPUT_PIN_B_1, 0);
	gpioWrite(OUTPUT_PIN_A_1, 1);

	exit(0);
}

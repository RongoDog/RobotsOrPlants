#include <pigpio.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#define MICRO_SEC_IN_SEC 1000000


#define INPUT_PIN 17


int main() {
	if (gpioInitialise() < 0) {
		fprintf(stderr, "Failed to initialize GPIO interface");
		exit(1);
	}

	gpioSetMode(INPUT_PIN, PI_INPUT);
	while (1) {
		fprintf(stdout, "Flame sensor is level %d\n", gpioRead(INPUT_PIN));
		gpioDelay(MICRO_SEC_IN_SEC);
	}

}

#include <pigpio.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#define MICRO_SEC_IN_SEC 1000000

int main() {
	if (gpioInitialise() < 0) {
		fprintf(stderr, "Failed to initialize GPIO interface");
		exit(1);
	}
	int handle;
	handle = i2cOpen(1, 0x40, 0);
	if (handle < 0) {
		if (handle == PI_BAD_I2C_BUS) {
			fprintf(stderr, "Bad I2C Bus\n");
		} else if (handle == PI_BAD_I2C_ADDR) {
			fprintf(stderr, "Bad address\n");
		} else if (handle == PI_NO_HANDLE) {
			fprintf(stderr, "No Handle\n");
		} else if (handle == PI_I2C_OPEN_FAILED) {
			fprintf(stderr, "Open failed\n");
		} else {
			fprintf(stderr, "Failed to open i2c bus, %d\n", handle);
		}
		exit(1);
	}

	int tempCode;
	while(1) { 

		tempCode = i2cReadWordData(handle, 0xE3);
		fprintf(stdout, "The code returned is %u\n", tempCode);
		fprintf(stdout, "The temperature is %d\n", (175.72*tempCode/65536) - 46.85);
		gpioDelay(MICRO_SEC_IN_SEC/2);
	}
}

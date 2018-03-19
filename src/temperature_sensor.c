#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <semaphore.h>
#include "temperature_sensor.h"
#include "pinout_definitions.h"

void *initialize_temperature_sensor(void *arg) {
	sem_t *i2c_semaphore = (sem_t *)arg;
	sem_wait(i2c_semaphore);
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
	sem_post(i2c_semaphore);

	int writeReturnCode;
	int msByte;
	int lsByte;
	double temp;
	while(1) { 
		sem_wait(i2c_semaphore);
		writeReturnCode = i2cWriteByte(handle, 0xF3);
		if (writeReturnCode < 0) {
			fprintf(stderr, "Write failed\n");
			exit(0);
		}
		sem_post(i2c_semaphore);
		gpioDelay(0.1 * MICRO_SEC_IN_SEC);
		sem_wait(i2c_semaphore);
		msByte = i2cReadByte(handle);
		if (msByte < 0) {
			fprintf(stderr, "Failed to read msByte\n");
			exit(0);
		}
		lsByte = i2cReadByte(handle);
		if (lsByte < 0) {
			fprintf(stderr, "Failed to read lsByte\n");
			exit(0);
		}
		sem_post(i2c_semaphore);
		fprintf(stdout, "The ms byte returned is %d\n", msByte);
		fprintf(stdout, "The ls byte returned is %d\n", lsByte);
		temp = (175.72 * (msByte * 256.0 + lsByte) / 65536.0) - 46.85;
		// Do something with value
		fprintf(stdout, "The temperature is %f\n", temp);
	}
}

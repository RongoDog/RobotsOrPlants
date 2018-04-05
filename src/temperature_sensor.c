#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/msg.h>
#include <math.h>
#include "temperature_sensor.h"
#include "pinout_definitions.h"

sem_t *i2c_semaphore;
int *message_queue_id;

void send_temperature_data(int temperature)
{
	struct sensor_message my_msg;
	struct sensor_data data;
	data.type = temperature_data;
	data.value = temperature;
	data.dir = not_specified;

	my_msg.data = data;
	my_msg.msg_key = SENSOR_MESSAGE;
	msgsnd(*message_queue_id, (void *)&my_msg, sizeof(data), IPC_NOWAIT);
}

void *initialize_temperature_sensor(void *arg) {
	struct thread_info *info = (struct thread_info *)arg;
	i2c_semaphore = info->semaphore;
	message_queue_id = info->message_queue_id;

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
		temp = (175.72 * (msByte * 256.0 + lsByte) / 65536.0) - 46.85;
		send_temperature_data((int)temp);
	}
}

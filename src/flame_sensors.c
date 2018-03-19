#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/msg.h>
#include "flame_sensors.h"
#include "pinout_definitions.h"

#define DEFAULT_ADDRESS 0x48
#define POINTER_CONVERSION 0x00
#define POINTER_CONFIG 0x01
#define POINTER_LOW_THRESHOLD 0x02
#define POINTER_HIGH_THRESHOLD 0x03

#define CONFIG_MUX_OFFSET 12

#define CONFIG_OS_SINGLE 0x8000
#define CONFIG_OS_CONTINUOUS 0x0000
#define CONFIG_GAIN 0x0600 // Specifies a gain of 4
#define CONFIG_MODE_SINGLE 0x0100 // Specifies single shot conversion
#define CONFIG_DATA_RATE 0x0080 // Specifies a data rate of 1600
#define DATA_RATE 1600

#define CONFIG_COMP_WINDOW_MODE 0x0010
#define CONFIG_COMP_ACTIVE_HIGH 0x0080
#define CONFIG_COMP_LATCHING 0x0004

#define CONFIG_COMP_QUE_DISABLE 0x0003

#define SINGLE_SHOT_READ_A0 0x83C3
#define SINGLE_SHOT_READ_A1 0x83D3
#define SINGLE_SHOT_READ_A2 0x83E3
#define SINGLE_SHOT_READ_A3 0x83F3

sem_t *i2c_semaphore;
int *message_queue_id;
struct direction_data current_direction;

void send_flame_data(double sensor_value, direction dir)
{
	struct sensor_message my_msg;
	struct sensor_data data;
	data.type = flame_data;
	data.value = sensor_value;
	data.dir = dir;

	my_msg.data = data;
	my_msg.msg_key = SENSOR_MESSAGE;
	msgsnd(*message_queue_id, (void *)&my_msg, sizeof(data), IPC_NOWAIT);
}

int receive_control_data(struct direction_data *dir)
{
    struct flame_sensor_message my_msg;
    if (msgrcv(*message_queue_id, (void *)&my_msg, sizeof(*dir), FLAME_SENSOR_CONTROL_MESSAGE, IPC_NOWAIT) == -1) {
        return(-1);
    }
    *dir = my_msg.data;
    return(0);
}

void handleWriteError(int returnVal) {
	if (returnVal == PI_BAD_HANDLE) {
		fprintf(stderr, "Bad handle\n");
	} else if (returnVal == PI_BAD_PARAM) {
		fprintf(stderr, "Bad parameter\n");
	} else if (returnVal == PI_I2C_WRITE_FAILED) {
		fprintf(stderr, "Write failed\n");
	}
}

void handleReadError(int returnVal) {
	if (returnVal == PI_BAD_HANDLE) {
		fprintf(stderr, "Bad handle\n");
	} else if (returnVal == PI_BAD_PARAM) {
		fprintf(stderr, "Bad parameter\n");
	} else if (returnVal == PI_I2C_READ_FAILED) {
		fprintf(stderr, "Read failed\n");
	}
}

// Opens the i2c bus to the ADC
int open_i2c_bus() { 
	int handle = i2cOpen(1, DEFAULT_ADDRESS, 0);
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
	}
	fprintf(stdout, "Successfully opened i2c connection to ADC. Handle: %d\n", handle);
	return handle;
}

int convert_to_integer(int low_bit, int high_bit) {
	// Convert to a 12-bit signed value
	int value = ((high_bit & 0xFF) << 4) | ((low_bit & 0xFF) >> 4);
	if ((value & 0x800) != 0) {
		value -= 1 << 4;
	}
	return value;
}

// This function applies a configuration to the
// ADS1015 ADC and then reads the conversion value. 
int read_sensor(int handle, direction mux, sem_t *i2c_semaphore) {
	
	int16_t config_value;
	switch(mux) {
		case front: 
			config_value = SINGLE_SHOT_READ_A0;
			break;
		case left: 
			config_value = SINGLE_SHOT_READ_A1;
			break;
		case right: 
			config_value = SINGLE_SHOT_READ_A2;
			break;
		case back: 
			config_value = SINGLE_SHOT_READ_A3;
			break;
		default: 
			config_value = SINGLE_SHOT_READ_A0;
	}

	sem_wait(i2c_semaphore);
	int returnVal = i2cWriteWordData(handle, POINTER_CONFIG, config_value);
	if (returnVal < 0) {
		handleWriteError(returnVal);
		fprintf(stderr, "Failed to write to configuration register\n");
		return -1;
	}
	sem_post(i2c_semaphore);
	gpioDelay(MICRO_SEC_IN_SEC/DATA_RATE + MICRO_SEC_IN_SEC/100);
	sem_wait(i2c_semaphore);
	returnVal = i2cReadWordData(handle, POINTER_CONVERSION);
	if (returnVal < 0) {
		handleReadError(returnVal);
		fprintf(stderr, "Failed to read from conversion register\n");
		return -1;
	}
	sem_post(i2c_semaphore);
	fprintf(stdout, "The direct return value is %d\n", returnVal);
	fprintf(stdout, "The conversion reg value is: [%x %x]\n", 
		(int)(0xFF & returnVal), (int)(0xFF & (returnVal >> 8)));

	return convert_to_integer(0xFF & (returnVal >> 8), 0xFF & returnVal);

}

void *initialize_flame_sensors(void *arg) {
	struct thread_info *info = (struct thread_info *)arg;
	i2c_semaphore = info->semaphore;
	message_queue_id = info->message_queue_id;

	// Set the default direction
	current_direction.value = not_specified; 

	sem_wait(i2c_semaphore);
	int handle = open_i2c_bus();
	if (handle < 0) {
		exit(1);
	}
	sem_post(i2c_semaphore);
	
	// This is the main while loop
	int converted_value;
	int next_direction = front; 
	while(1) { 
		converted_value = read_sensor(handle, next_direction, i2c_semaphore);
		send_flame_data(converted_value, next_direction);
		receive_control_data(&current_direction);
		if (current_direction.value == not_specified) {
			next_direction = ((int)next_direction + 1)%4; 
		} else {
			next_direction = current_direction.value;
		}
	}
	exit(0);

}

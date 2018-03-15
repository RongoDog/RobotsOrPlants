#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "flame_sensors.h"
#include "robot_i2c.h"

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
int read_sensor(int handle, int mux) {
	
	int16_t config_value;
	switch(mux) {
		0: SINGLE_SHOT_READ_A0;
		1: SINGLE_SHOT_READ_A1;
		2: SINGLE_SHOT_READ_A2;
		3: SINGLE_SHOT_READ_A3;
		default: SINGLE_SHOT_READ_A0;
	}
	int returnVal = i2cWriteWordData(handle, POINTER_CONFIG, config_value);
	if (returnVal < 0) {
		handleWriteError(returnVal);
		fprintf(stderr, "Failed to write to configuration register\n");
		return -1;
	}
	gpioDelay(MICRO_SEC_IN_SEC/DATA_RATE + MICRO_SEC_IN_SEC/100);

	returnVal = i2cReadWordData(handle, POINTER_CONVERSION);
	if (returnVal < 0) {
		handleReadError(returnVal);
		fprintf(stderr, "Failed to read from conversion register\n");
		return -1;
	}
	fprintf(stdout, "The direct return value is %d\n", returnVal);
	fprintf(stdout, "The conversion reg value is: [%x %x]\n", 
		(int)(0xFF & returnVal), (int)(0xFF & (returnVal >> 8)));

	return convert_to_integer(0xFF & (returnVal >> 8), 0xFF & returnVal);

}

void *initialize_flame_sensors(void *arg) {

	int handle = open_i2c_bus();
	if (handle < 0) {
		exit(1);
	}
	
	// This is the main while loop
	int current_sensor = 0;
	while(1) { 
		int converted_value = read_sensor(handle, current_sensor);
		current_sensor = (current_sensor + 1)%4; 
	}
	exit(0);

}

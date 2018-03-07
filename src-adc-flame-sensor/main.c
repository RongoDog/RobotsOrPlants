#include <pigpio.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#define MICRO_SEC_IN_SEC 1000000

#define NOP 0x00 // No operation
#define ADC_SEQUENCE_REGISTER 0x02 // Selects ADCs for conversion
#define GENERAL_PURPOSE_CONTROL_REGISTER 0x03 // DAC and ADC control register
#define ADC_PIN_CONFIGURATION 0x04 // Selects which pins are ADC inputs
#define DAC_PIN_CONFIGURATION 0x05 // Selects which pins are DAC outputs
#define PULL_DOWN_CONFIGURATION 0x06 // Selects which pins have 85kohm pull down
#define LDAC_MODE 0x07 // Selects the operation of the load DAC
#define GPIO_WRITE_CONFIGURATION 0x08 // Selects which pins are general purpose outputs
#define GPIO_WRITE_DATA 0x09 // Writes data to general-purpose outputs. 
#define GPIO_READ_CONFIGURATION 0x0A // Selects which pins are general purpose inputs
#define POWER_DOWN_REFERENCE_CONTROL 0x0B // Powers down the DACS and controls the reference
#define THREE_STATE_PINS 0x0C // Selects which pins are three stated
#define RESET_REGISTER 0x0F // Allow a software reset

#define LSB_PIN 4
#define RESET_PIN 17

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
	int handle = i2cOpen(1, 0b0010001, 0);
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
	return handle;
}

// Selects all pins except a0 to be used at pull down config
int select_pull_down_resistor_pins(int handle) {
	// Writting address a0 to be used for analog to digital conversion. 
	int returnVal = i2cWriteWordData(handle, PULL_DOWN_CONFIGURATION, 0x00FE);
	if (returnVal < 0) {
		if (returnVal == PI_BAD_HANDLE) {
			fprintf(stderr, "Bad handle\n");
		} else if (returnVal == PI_BAD_PARAM) {
			fprintf(stderr, "Bad parameter\n");
		} else if (returnVal == PI_I2C_WRITE_FAILED) {
			fprintf(stderr, "Write failed\n");
		}
		fprintf(stderr, 
				"Failed to make a0 not a pull down resistor configuration, return: %d\n", returnVal);
	}
	return returnVal;
}

// Configures the general purpose control register to enable the ADC buffer
int buffer_enable(int handle) {

	char buffer_enable_precharge_bytes[] = {GENERAL_PURPOSE_CONTROL_REGISTER, 0x03, 0x00};
	int returnVal = i2cWriteDevice(handle, buffer_enable_precharge_bytes, 3);
	if (returnVal < 0) {
		handleWriteError(returnVal);
		fprintf(stderr, "Failed to write general purpose control register pointer value\n");
		return -1; 
	}
	return 0;

}

// Selects pin a0 to be used as ADC
int select_adc_pins(int handle) {
	// Writting address a0 to be used for analog to digital conversion. 
	int returnVal = i2cWriteWordData(handle, ADC_PIN_CONFIGURATION, 0x0001);
	if (returnVal < 0) {
		if (returnVal == PI_BAD_HANDLE) {
			fprintf(stderr, "Bad handle\n");
		} else if (returnVal == PI_BAD_PARAM) {
			fprintf(stderr, "Bad parameter\n");
		} else if (returnVal == PI_I2C_WRITE_FAILED) {
			fprintf(stderr, "Write failed\n");
		}
		fprintf(stderr, 
				"Failed to write address a0 for analog to digital conversion, return: %d\n", returnVal);
	}
	return returnVal;
}

int main() {
	if (gpioInitialise() < 0) {
		fprintf(stderr, "Failed to initialize GPIO interface");
		exit(1);
	}

	gpioSetMode(LSB_PIN, PI_OUTPUT);
	gpioWrite(LSB_PIN, 1);
	gpioWrite(RESET_PIN, 0);

	gpioSetMode(RESET_PIN, PI_OUTPUT);
	gpioWrite(RESET_PIN, 1);
	gpioDelay(100000);
	gpioWrite(RESET_PIN, 0);
	gpioDelay(100000);
	gpioWrite(RESET_PIN, 1);
	
	open_i2c_bus();
	int handle = open_i2c_bus();
	if (handle < 0) {
		exit(1);
	}

	int returnVal;

	returnVal = i2cReadWordData(handle, GENERAL_PURPOSE_CONTROL_REGISTER);
	if (returnVal < 0) {
		handleReadError(returnVal);
		fprintf(stderr, "Failed to read general purpose register\n");
		exit(1);
	}
	fprintf(stdout, "The general purpose control reg value is: %d\n", returnVal);

	returnVal = i2cWriteWordData(handle, GENERAL_PURPOSE_CONTROL_REGISTER, 0x0003);
	if (returnVal < 0) {
		handleWriteError(returnVal);
		fprintf(stderr, "Failed to write to general purpose control register\n");
		exit(1);
	}

	returnVal = i2cReadWordData(handle, GENERAL_PURPOSE_CONTROL_REGISTER);
	if (returnVal < 0) {
		handleReadError(returnVal);
		fprintf(stderr, "Failed to read general purpose register\n");
		exit(1);
	}
	fprintf(stdout, "The general purpose control reg value is: %d\n", returnVal);

	
	/*
	gpioDelay(1000000);
	char bytes[] = {0x0F, 0x0D, 0xAC}; 
	int returnVal = i2cWriteDevice(handle, bytes, 3);
	if (returnVal < 0) {
		handleWriteError(returnVal);
		fprintf(stderr, "Software reset failed\n");
		exit(1);
	}
	*/

	exit(0);

	/*
	if (buffer_enable(handle) < 0) {
		exit(1);
	}
	*/
	/*
	if (select_adc_pins(handle) < 0) {
		exit(1);
	}

	
	int returnVal = i2cWriteWordData(handle, 0x02, 0x0001);
	if (returnVal < 0) {
		if (returnVal == PI_BAD_HANDLE) {
			fprintf(stderr, "Bad handle\n");
		} else if (returnVal == PI_BAD_PARAM) {
			fprintf(stderr, "Bad parameter\n");
		} else if (returnVal == PI_I2C_WRITE_FAILED) {
			fprintf(stderr, "Write failed\n");
		}
		fprintf(stderr, 
				"Failed to write sequence register a0 for analog to digital conversion, return: %d\n", returnVal);
		exit(1);
	}

	returnVal = i2cWriteWordData(handle, 0x0B, 0x0200);
	if (returnVal < 0) {
		if (returnVal == PI_BAD_HANDLE) {
			fprintf(stderr, "Bad handle\n");
		} else if (returnVal == PI_BAD_PARAM) {
			fprintf(stderr, "Bad parameter\n");
		} else if (returnVal == PI_I2C_WRITE_FAILED) {
			fprintf(stderr, "Write failed\n");
		}
		fprintf(stderr, 
				"Failed to write value to enable voltage ref, return: %d\n", returnVal);
		exit(1);
	}
	returnVal = i2cWriteByte(handle, 0x40);
	if (returnVal < 0) {
		if (returnVal == PI_BAD_HANDLE) {
			fprintf(stderr, "Bad handle\n");
		} else if (returnVal == PI_BAD_PARAM) {
			fprintf(stderr, "Bad parameter\n");
		} else if (returnVal == PI_I2C_WRITE_FAILED) {
			fprintf(stderr, "Write failed\n");
		}
		fprintf(stderr, 
				"Failed to write mystery bit, return: %d\n", returnVal);
		exit(1);
	}

 	int msb;
 	int lsb;
	while(1) { 
		msb = i2cReadByte(handle);
		lsb = i2cReadByte(handle);
		fprintf(stdout, "The msb returned is %d\n", msb);
		fprintf(stdout, "lhe msb returned is %d\n", lsb);
		gpioDelay(MICRO_SEC_IN_SEC/2);
	}
	*/
}

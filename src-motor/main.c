#include <pigpio.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>

//this is the left motor
#define OUTPUT_PIN_E_1 14
#define OUTPUT_PIN_B_1 15
#define OUTPUT_PIN_A_1 18

//this is the right motor
#define OUTPUT_PIN_E_2 17
#define OUTPUT_PIN_B_2 27
#define OUTPUT_PIN_A_2 22

#define MICRO_SEC_IN_SEC 1000000

typedef struct time_tracker {
	long int start;
	long int end;
	long int current_distance;
} time_tracker;

bool MotorsInit(){ //returns 1 if failure to initialize
	
	if (gpioInitialise() < 0) {
		fprintf(stderr, "Failed to initialize GPIO interface");
		return 1;
	}
	gpioSetMode(OUTPUT_PIN_E_1, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_B_1, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_A_1, PI_OUTPUT);
	
	gpioSetMode(OUTPUT_PIN_E_2, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_B_2, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_A_2, PI_OUTPUT);
	
	gpioWrite(OUTPUT_PIN_E_1, 0);
	gpioWrite(OUTPUT_PIN_E_2, 0);
	
	return 0;
	
}

void SlowLeft(){
	
	gpioWrite(OUTPUT_PIN_E_1, 1);
	gpioWrite(OUTPUT_PIN_E_2, 1);
	
	//turn right motor forwards
	gpioWrite(OUTPUT_PIN_B_2, 0);
	gpioWrite(OUTPUT_PIN_A_2, 1);
		
}

void SlowRight(){
	
	gpioWrite(OUTPUT_PIN_E_1, 1);
	gpioWrite(OUTPUT_PIN_E_2, 1);
	
	//turn left motor forwards
	gpioWrite(OUTPUT_PIN_B_1, 0);
	gpioWrite(OUTPUT_PIN_A_1, 1);
		
}

void SharpLeft(){
	
	gpioWrite(OUTPUT_PIN_E_1, 1);
	gpioWrite(OUTPUT_PIN_E_2, 1);
	
	//turn left motor backwards
	gpioWrite(OUTPUT_PIN_B_1, 1);
	gpioWrite(OUTPUT_PIN_A_1, 0);
	
	//turn right motor forwards
	gpioWrite(OUTPUT_PIN_B_2, 0);
	gpioWrite(OUTPUT_PIN_A_2, 1);
		
}

void SharpRight(){
	
	gpioWrite(OUTPUT_PIN_E_1, 1);
	gpioWrite(OUTPUT_PIN_E_2, 1);
	
	//turn left motor forwards
	gpioWrite(OUTPUT_PIN_B_1, 0);
	gpioWrite(OUTPUT_PIN_A_1, 1);
	
	//turn right motor backwards
	gpioWrite(OUTPUT_PIN_B_2, 1);
	gpioWrite(OUTPUT_PIN_A_2, 0);
		
}

void DriveStraight(){
	
	gpioWrite(OUTPUT_PIN_E_1, 1);
	gpioWrite(OUTPUT_PIN_E_2, 1);
	
	gpioWrite(OUTPUT_PIN_B_1, 0);
	gpioWrite(OUTPUT_PIN_A_1, 1);
	
	gpioWrite(OUTPUT_PIN_B_2, 0);
	gpioWrite(OUTPUT_PIN_A_2, 1);

}

void DriveBack(){
	
	gpioWrite(OUTPUT_PIN_E_1, 1);
	gpioWrite(OUTPUT_PIN_E_2, 1);
	
	gpioWrite(OUTPUT_PIN_B_1, 1);
	gpioWrite(OUTPUT_PIN_A_1, 0);
	
	gpioWrite(OUTPUT_PIN_B_2, 1);
	gpioWrite(OUTPUT_PIN_A_2, 0);

}

void StopMotors(){
	
	gpioWrite(OUTPUT_PIN_E_1, 0);
	gpioWrite(OUTPUT_PIN_E_2, 0);

}

int main() {
	if (gpioInitialise() < 0) {
		fprintf(stderr, "Failed to initialize GPIO interface");
		exit(1);
	}
	gpioSetMode(OUTPUT_PIN_E_1, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_B_1, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_A_1, PI_OUTPUT);
	
	gpioSetMode(OUTPUT_PIN_E_2, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_B_2, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_A_2, PI_OUTPUT);

	gpioWrite(OUTPUT_PIN_E_1, 1);
	gpioWrite(OUTPUT_PIN_B_1, 0);
	gpioWrite(OUTPUT_PIN_A_1, 1);

	gpioWrite(OUTPUT_PIN_E_2, 1);
	gpioWrite(OUTPUT_PIN_B_2, 0);
	gpioWrite(OUTPUT_PIN_A_2, 1);


	gpioDelay(MICRO_SEC_IN_SEC * 2);

	gpioWrite(OUTPUT_PIN_B_1, 1);
	gpioWrite(OUTPUT_PIN_A_1, 0);
	gpioWrite(OUTPUT_PIN_B_2, 1);
	gpioWrite(OUTPUT_PIN_A_2, 0);

	gpioDelay(MICRO_SEC_IN_SEC * 2);

	gpioWrite(OUTPUT_PIN_E_1, 0);
	gpioWrite(OUTPUT_PIN_E_2, 0);
	exit(0);
}

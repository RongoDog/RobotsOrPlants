#ifndef Motor__h
#define Motor_h

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

bool MotorsInit();

void SlowLeft();

void SlowRight();

void SharpLeft();

void SharpRight();

void DriveStraight();

void DriveBack();

void StopMotors();

#endif


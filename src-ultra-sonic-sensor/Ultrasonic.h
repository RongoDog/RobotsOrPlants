#ifndef Ultrasonic_h /* include guard */
#define Ultrasonic_h
 
#include <pigpio.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>

#define OUTPUT_PIN 23
#define INPUT_PIN 24

#define MICRO_SEC_IN_SEC 1000000
#define USECOND_CENTIMETER_RATIO 58

#define MIN_RANGE_LENGTH 2
#define MAX_RANGE_LENGTH 500

typedef struct time_tracker {
	long int start;
	long int end;
	long int current_distance;
} time_tracker;


void callback_function(int gpio, int level, unsigned int tick, void *t);
 
#endif

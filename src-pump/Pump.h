#ifndef PUMP_H /* Include guard */
#define PUMP_H_

#include <pigpio.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>

#define OUTPUT_PIN_E 14
#define OUTPUT_PIN_B 15
#define OUTPUT_PIN_A 18

#define MICRO_SEC_IN_SEC 1000000

bool initializePump();
void DrivePump();
void StopDrivingPump();

#endif // PUMP_H_


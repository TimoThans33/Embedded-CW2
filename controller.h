#ifndef _controller_h_
#define _controller_h_

#include "mbed.h"
#include "rtos.h"
#include "message.h"
#include "decode.h"

//Photointerrupter input pins
#define I1pin D3
#define I2pin D6
#define I3pin D5

//Incremental encoder input pins
#define CHApin   D12
#define CHBpin   D11

//Motor Drive output pins   //Mask in output byte
#define L1Lpin D1           //0x01
#define L1Hpin A3           //0x02
#define L2Lpin D0           //0x04
#define L2Hpin A6          //0x08
#define L3Lpin D10           //0x10
#define L3Hpin D2          //0x20

#define PWMpin D9

//Motor current sense
#define MCSPpin   A1
#define MCSNpin   A0

//Test outputs
#define TP0pin D4
#define TP1pin D13
#define TP2pin A2


#define PWM_LIMIT 1000
#define VEL_CONST 65
#define POS_CONST 20


void motorOut(int8_t driveState);

extern void ISRPhotoSensors();

extern void motorHome();

extern void driveISR();

extern void motorCtrlFn();

extern void motorCtrlTick();

uint32_t velocityController();

uint32_t positionController();

extern void PWMPeriod(int32_t period);

extern void PWMTorque(uint32_t torque);

extern Thread controllerThread;

extern volatile int32_t rotorPosition;

#endif

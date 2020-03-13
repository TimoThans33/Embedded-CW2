#ifndef _message_h_
#define _message_h_

#include "rtos.h"
#include "mbed.h"

 // The output codes
#define START  0
#define MOTOR  1
#define VELOCITY 2
#define SET_VELOCITY 3
#define ROTOR 4
#define SEC 12
#define HIT 11


extern RawSerial pc;

typedef struct {
   uint8_t command;
   int32_t data;
 }message_struct;

void getMail();

extern void setMail(uint8_t command, int32_t data);

#endif

#ifndef _message_h_
#define _message_h_
#include "rtos.h"
#include "mbed.h"

 // The output codes
#define START  0


extern RawSerial pc;

typedef struct {
   uint8_t command;
   int32_t data;
 }message_struct;

void getMail();

extern void setMail(uint8_t command, int32_t data);

#endif

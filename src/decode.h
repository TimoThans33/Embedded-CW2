#ifndef _decode_h_
#define _decode_h_

#include "mbed.h"
#include "rtos.h"
#include "message.h"
#include "controller.h"

extern volatile int velTarget;

extern volatile float rotTarget;

extern volatile uint64_t newKey;

extern volatile bool newKeyAdded;

extern volatile uint16_t frequency[4];

extern volatile bool newTone;

extern Mutex newKey_mutex;

void serialISR();

extern void decode();

#endif

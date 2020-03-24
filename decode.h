#ifndef _decode_h_
#define _decode_h_

#include "mbed.h"
#include "rtos.h"
#include "message.h"
#include "controller.h"

extern volatile float velTarget;

extern volatile float rotTarget;

extern volatile uint64_t newKey;

extern volatile bool newKeyAdded;

extern volatile char tone[18];

extern volatile bool newTone;

extern Mutex newKey_mutex;

void serialISR();

extern void decode();

#endif

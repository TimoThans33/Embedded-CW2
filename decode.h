#ifndef _decode_h_
#define _decode_h_

#include "mbed.h"
#include "rtos.h"
#include "message.h"
#include "controller.h"

extern volatile float velTarget;

extern volatile uint64_t newKey;

extern Mutex newKey_mutex;

void serialISR();

extern void decode();

#endif

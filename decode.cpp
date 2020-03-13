#include "decode.h"


Queue<uint8_t, 8> inCharQ;

char charbuf[17];

volatile float velTarget = 0.0;

int _count;
volatile uint64_t newKey;

Mutex newKey_mutex;




void serialISR(){
  uint8_t newChar = pc.getc();
  inCharQ.put(newChar);
}


void decode(void){
  // Attach the ISR to serial port events
  pc.attach(&serialISR);
  int counter = 0;
 /*
  while (1){

    osEvent newEvent = inCharQ.get();
    uint8_t newChar = (uint8_t)newEvent.value.p;

    if(counter >18){
      counter = 0;
    }
    charbuf[counter] = newChar;

    if(newChar == '\r'){
      charbuf[counter] = '\0';
      counter = 0;
      switch (charbuf[0]) {

        case 'V':
          sscanf(charbuf, "V%f", &vel_target);
          setMail(SET_VELOCITY, vel_target);

        case 'K':
        newKey_mutex.lock();
        // Read formatted input from a string
        sscanf(charbuf,"K%x",&newKey);

        newKey_mutex.unlock();
      }

    }
    else{
      counter++;
    }
  }
  */
}

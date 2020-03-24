#include "decode.h"


Queue<char, 8> inCharQ;

char charbuf[17];

float setRotTarget = 0;
volatile float velTarget = 0.0;
volatile float rotTarget = 0.0;

int _count;
volatile uint64_t newKey;
volatile bool newKeyAdded;

Mutex newKey_mutex;

volatile char tone[18];
volatile bool newTone;


void serialISR(){
  char newChar = pc.getc();
  inCharQ.put((char*)newChar);
}


void decode(void){
  // Attach the ISR to serial port events
  pc.attach(&serialISR);
  int counter = 0;


  while (1){

    osEvent newEvent = inCharQ.get();
    char newChar = *((char*)newEvent.value.p);

    if(counter >18){
      counter = 0;
    }
    charbuf[counter] = newChar;

    if(newChar == '\r'){
      charbuf[counter] = '\0';
      counter = 0;
      switch (charbuf[0]) {

        case 'V':
          sscanf(charbuf, "V%f", &velTarget);
          setMail(SET_VELOCITY,  *(int32_t*)&velTarget);

          break;
        case 'R':
          sscanf(charbuf, "R%f", &setRotTarget);
          setMail(SET_ROTATION,  *(int32_t*)&setRotTarget);
          rotTarget = rot + setRotTarget;
          break;
        case 'K':
          newKey_mutex.lock();
          // Read formatted input from a string
          sscanf(charbuf,"K%x", &newKey);
          setMail(KEY_UPPER, (uint32_t)((newKey>>32)&0xFFFFFFFF));
          newKey_mutex.unlock();
          newKeyAdded = true;
          break;
        case 'T':
          //sscanf(charbuf, "T%s", &tone);
          //setMail(TONE, *(int32_t*)&tone);
          newTone = true;
      }

    }
    else{
      counter++;
    }
  }
}

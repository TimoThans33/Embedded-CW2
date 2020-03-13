#include "decode.h"


Queue<char, 8> inCharQ;

char charbuf[17];

volatile float velTarget = 2.0;
volatile float rotTarget = 2.0;

int _count;
volatile uint64_t newKey;

Mutex newKey_mutex;




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
          setMail(SET_VELOCITY, velTarget);

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

}

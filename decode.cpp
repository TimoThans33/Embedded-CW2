#include "decode.h"

typedef struct {
  uint8_t input;
} mail_t;

//we should be abe to use Queue here as we are only using pointers but it doesn't seem to work
//So now use mail instead and define a uint8_t (does not have to be a pointer anymore)
Mail<mail_t, 32> inCharQ;


uint8_t charbuf[17];

float setRotTarget = 0;
volatile float velTarget = 0.0;
volatile float rotTarget = 0.0;

int _count;
volatile uint64_t newKey;
volatile bool newKeyAdded;

Mutex newKey_mutex;

volatile char tone[50];
volatile bool newTone;


void serialISR(){
  //Allocate a block from the memory for this mail
  mail_t *mail = inCharQ.alloc();
  //get serial input
  mail->input = pc.getc();
  //Put the pointer to the respective memory block in the queue
  inCharQ.put(mail);
}


void decode(void){
  // Attach the ISR to serial port events
  pc.attach(&serialISR);
  int counter = 0;
  while (1){
    osEvent newEvent = inCharQ.get();
    //check for serial input...
    if (newEvent.status == osEventMail){
      mail_t *mail = (mail_t*)newEvent.value.p;
      //check for overflow of charbuf
      if(counter > 18){
        counter = 0;
      }
      //store serial input in buffer
      charbuf[counter] = mail->input;
      //counter += 1;
      inCharQ.free(mail);

      //Begin decoding
      if(charbuf[counter] == '\r')
      {
        charbuf[counter] = '\0';
        switch(charbuf[0]){
          // velocity
          case 'V':
            sscanf((char*)charbuf, "V%f", &velTarget);
            setMail(SET_VELOCITY,  velTarget);
            break;
          // rotation
          case 'R':
            sscanf((char*)charbuf, "R%f", &setRotTarget);
            setMail(SET_ROTATION,  setRotTarget);
            rotTarget = rot + setRotTarget;
            break;
          //key
          case 'K':
            newKey_mutex.lock();
            sscanf((char*)charbuf, "K%x",&newKey);
            newKey_mutex.unlock();
            setMail(KEY, (uint64_t)(newKey&0xFFFFFFFF));
            newKeyAdded = true;
            break;
          // melody
          case 'T':
            sscanf((char*)charbuf, "T%s", tone);
            setMail(TONE, *(int32_t*)&tone);
            newTone = true;
            break;
        }
        counter = 0;
        for (int i=0; i<18; i++){
          charbuf[i] = NULL;
      }
    }
      else{
        counter++;
      }
    }
  }
}

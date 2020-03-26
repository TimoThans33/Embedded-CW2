#include "decode.h"

typedef struct {
  uint8_t input;
} mail_t;

//we should be abe to use Queue here as we are only using pointers but it doesn't seem to work
//So now use mail instead and define a uint8_t (does not have to be a pointer anymore)
Mail<mail_t, 16> inCharQ;


char charbuf[18];

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
  //uint8_t test = 'V';
  //pc.printf("input:  %d == %d\r\n", inputs, test);
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
      counter += 1;
      inCharQ.free(mail);
      //convert to int so compiler can read from ASCII table and print characters
      unsigned char k = (int)charbuf[0];
      unsigned char q = (int)charbuf[1];
      unsigned char i = (int)charbuf[2];
      unsigned char f = (int)charbuf[3];
      pc.printf("input:  %c & %c & %c & %c \n", k, q, i, f);

      //Begin decoding
      if(mail->input == '\r')
      {
        charbuf[counter-1] = '\0';
        counter = 0;
        switch(charbuf[1]){
          //key
          case 'k':
                newKey_mutex.lock();
                sscanf(charbuf, "K%x",&newKey);
                newKey_mutex.unlock();
                setMail(KEY, (uint64_t)(newKey&0xFFFFFFFF));
                newKeyAdded = true;
                break;
        }
      }
    }
    /*
    if(intChar == '\r'){
      charbuf[counter] = '\0';
      counter = 0;
      //setMail(ERROR,  &charbuf);
      //setMail(ERROR, charbuf[0]);

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
          sscanf(charbuf, "T%s", tone);
          setMail(TONE, *(int32_t*)&tone);
          newTone = true;
          break;
      }

    }
    else{
      counter++;
    }
*/
  }
}

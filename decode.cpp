#include "decode.h"

typedef struct {
  uint8_t input;
} mail_t;

//we should be abe to use Queue here as we are only using pointers but it doesn't seem to work
//So now use mail instead and define a uint8_t (does not have to be a pointer anymore)
Mail<mail_t, 8> inCharQ;


uint8_t charbuf[17];

//initiate the global variables
volatile float setRotTarget = 0;
volatile float velTarget = 0.0;
volatile float rotTarget = 0.0;

volatile uint64_t newKey;
volatile bool newKeyAdded;

volatile uint8_t tone;
volatile bool newTone;

volatile uint32_t frequency;

//Define frequencies of some notes.. check ASCII table
uint8_t notes[7] = {67, 68, 69, 70, 71, 65, 66};
uint8_t notes_flat[5] = {67, 68, 70, 71, 65};
int octaves_flat[3][5] = {{34, 38, 46, 52, 58},
                          {69, 77, 92, 104, 116},
                          {138, 155, 185, 208, 233}};
int octaves[3][7] = {{32, 36, 41, 43, 49, 55, 61},
                     {65, 73, 82, 87, 98, 110, 123},
                     {130, 146, 164, 174, 196, 220, 246}};


//initial local variables
int _count;
int octave;
char unknown;

//initiate classes
Mutex newKey_mutex;

//****************************FUNCTION PROTOTYPES*****************************
void NoteToFreq(void);
void serialISR(void);
//*****************************************************************************

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
            sscanf((char*)charbuf, "k%x",&newKey);
            newKey_mutex.unlock();
            //Same as KEY_UPPER and KEY_LOWER implementations
            setMail(KEY, (uint64_t)(newKey&0xFFFFFFFF));
            newKeyAdded = true;
            break;
          case 'T':
            //Playing user defined notes
            sscanf((char*)charbuf, "T%c %c %d",&tone,&unknown,&octave);
            printf("Tone: %d\r\n",tone);
            NoteToFreq();
            //print for debugging purposes
            setMail(TONE, frequency);
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


void NoteToFreq(void){
  int index;
  //check for flat notes
  if ((int) unknown == 35){
    //Iterate through data to find matching flat note
    for (int i = 0; i < 7; i++){
      if ( (int) tone == (int) notes_flat[i])
      {
        index = i;
      }
    }
    //Take the frequency for user defined octave
    frequency = (int) octaves_flat[octave-1][index];
  }
  //Normal notes
  else{
    for (int i = 0; i < 7; i++){
      //Iterate through data to find matching note
      if( (int) tone == (int) notes[i]){
        index = i;
      }
    }
      //Take the frequency for user defined octave
      frequency = (int) octaves[(int)unknown-49][index];
    }
}

void serialISR(){
  //Allocate a block from the memory for this mail
  mail_t *mail = inCharQ.alloc();
  //get serial input
  mail->input = pc.getc();
  //Put the pointer to the respective memory block in the queue
  inCharQ.put(mail);
}

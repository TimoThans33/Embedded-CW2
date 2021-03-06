#include "decode.h"

//we should be abe to use Queue here as we are only using pointers to the ASCII table but it doesn't seem to work
//So now use mail instead (which uses both pointer and memory block)
//and make a construct this however might occupy more resources
typedef struct {
  uint8_t input;
} mail_t;

Mail<mail_t, 8> inCharQ;



//initiate the global variables
volatile int velTarget = 0;
volatile float rotTarget = 0.0;

volatile uint64_t newKey;
volatile bool newKeyAdded;

volatile uint16_t frequency[4];
volatile bool newTone;

//Define frequencies of some notes.. check ASCII table
uint8_t notes[7] = {67, 68, 69, 70, 71, 65, 66};
uint8_t notes_flat[5] = {67, 68, 70, 71, 65};
int octaves_flat[5][5] = {{34, 38, 46, 52, 58},
                          {69, 77, 92, 104, 116},
                          {138, 155, 185, 208, 233},
                          {277, 311, 370, 415, 466},
                          {555, 622, 740, 830, 932}};
int octaves[5][7] = {{32, 36, 41, 43, 49, 55, 61},
                     {65, 73, 82, 87, 98, 110, 123},
                     {130, 146, 164, 174, 196, 220, 246},
                     {262, 294, 330, 350, 392, 440, 494},
                     {523, 587, 659, 698, 783, 880, 987}};


//initialtelocal variables
int _count;
int octave;
char unknown;
uint8_t tone;
uint8_t charbuf[17];
int setRotTarget_low;
int setRotTarget_high;
int setRotTarget;

//initiate classes
Mutex newKey_mutex;

//****************************FUNCTION PROTOTYPES*****************************
void NoteToFreq(void);
void serialISR(void);
//*****************************************************************************

//decoding serial command (main function of thread)
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
      if(mail->input != '\r'){
        setMail(SERIAL, (char) mail->input);
      }
      inCharQ.free(mail);
      //Begin decoding
      if(charbuf[counter] == '\r')
      {
        charbuf[counter] = '\0';
        switch(charbuf[0]){
          // velocity
          case 'V':
            sscanf((char*)charbuf, "V%d", &velTarget);
            setMail(SET_VELOCITY,  velTarget);
            break;
          // rotation
          case 'R':
            sscanf((char*)charbuf, "R-%d.%d",&setRotTarget_high,&setRotTarget_low);
            setRotTarget =  setRotTarget_high*10 + setRotTarget_low;
            setMail(SET_ROTATION_UP, setRotTarget_high);
            setMail(SET_ROTATION_DOWN, setRotTarget_low);
            rotTarget = rot + setRotTarget/10.0;
            break;
          // Key
          case 'K':
            newKey_mutex.lock();
            sscanf((char*)charbuf, "k%x",&newKey);
            newKey_mutex.unlock();
            //Same as KEY_UPPER and KEY_LOWER implementations
            setMail(KEY, (uint64_t)(newKey&0xFFFFFFFF));
            newKeyAdded = true;
            break;
          // Melody
          case 'T':
            NoteToFreq();
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
  int counter = 0;
  for (int i=1; i<18; i++){
    //check for first number (then we know that a note is finished)
    if((int) charbuf[i] <= 57 && charbuf[i] >= 49){ //ASCII table
      //Check for #
      if((int) charbuf[i-1] == 35){
        //iterate through data to find matching flat note
        for (int k = 0; k < 5; k++){
          if ((int) charbuf[i-2] == (int) notes_flat[k]){
            index = k;
          }
          //make sure the program does not crash when user makes typo
          else{break;}
        }
        frequency[counter] = octaves_flat[(int)(charbuf[i])-49][index];
      }
      else{
        //Iterate through data to find matching note
        for (int k=0; k<7; k++){
          if ((int) charbuf[i-1] == (int) notes[k]){
            index = k;
          }
          //make sure the program does not crash when user makes typo
          else{break;}
        }
        frequency[counter] = octaves[(int)(charbuf[i])-49][index];
      }
      counter++;
    }
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

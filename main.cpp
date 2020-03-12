#include "mbed.h"
#include "SHA256.h"
#include "Thread.h"
#include "rtos.h"
#include "message.h"
#include "controller.h"





//Mapping from sequential drive states to motor phase outputs
/*
State   L1  L2  L3
0       H   -   L
1       -   H   L
2       L   H   -
3       L   -   H
4       -   L   H
5       H   L   -
6       -   -   -
7       -   -   -
*/

//const int8_t stateMap[] = {0x07,0x01,0x03,0x02,0x05,0x00,0x04,0x07}; //Alternative if phase order of input or drive is reversed



// Declare and initialise the input sequency, key, nonce and hash
uint8_t sequence[] = {0x45,0x6D,0x62,0x65,0x64,0x64,0x65,0x64,
                      0x20,0x53,0x79,0x73,0x74,0x65,0x6D,0x73,
                      0x20,0x61,0x72,0x65,0x20,0x66,0x75,0x6E,
                      0x20,0x61,0x6E,0x64,0x20,0x64,0x6F,0x20,
                      0x61,0x77,0x65,0x73,0x6F,0x6D,0x65,0x20,
                      0x74,0x68,0x69,0x6E,0x67,0x73,0x21,0x20,
                      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

uint64_t* key = (uint64_t*)&sequence[48];
uint64_t* nonce = (uint64_t*)&sequence[56];
uint8_t hash2[32];


//initiate pointer
int pointer = 0;




// Declarations
volatile int _count;


//**************************Function prototypes********************************
//void serialISR(void);
//void serial_queue(void);
//void computeHash(void);
//*****************************************************************************

Mutex key_mutex;
// Create a global instance of class Queue
//Queue<uint8_t, 8> inCharQ;

//Thread decodethread;
Thread messagethread;
Thread controllerThread;

int main()
{
    setMail(START, 13);

    ISRPhotoSensors();
    //PWMPeriod(2000);

    messagethread.start(getMail);
    //decodethread.start(callback(serial_queue));
    controllerThread.start(driveCtrl);
    //Run the motor synchronisation
    //orState is subtracted from future rotor state inputs to align rotor and motor states
    //Poll the rotor state and set the motor outputs accordingly to spin the motor


    Timer t;
    t.start();
    uint8_t HashCount = 0;
    *nonce = 0;
    *key = 0;
    while (true) {
      /*
      SHA256::computeHash(hash2, sequence, 64);
      if ((hash2[0]==0) && (hash2[1]==0)) {
              setMail(*nonce, HashCount);
      }
      HashCount += 1;
      if (t >= 1){
        setMail(*nonce, HashCount);
        HashCount = 0;
        t.reset();
      }
      *nonce+=1;
      */
    }
}











/*
void serialISR(){
  uint8_t* newChar = inCharQ.alloc();
  *newChar = pc.getc();
  inCharQ.put(newChar);
}

void serial_queue(void){
  pc.attach(&serialISR);
  while (1){
    osEvent newEvent = inCharQ.get();
    uint8_t* newChar = (uint8_t*)newEvent.value.p;
    charbuf[counter] = newChar;
    if(newChar == /r){
      switch(charbuf[0]){
      case 'K':
              key_mutex.lock();
              sscanf(charbuf, K, &new_key);
              key_mutex.unlock();
              setMail(KEY, new_key);
    }
  }
    inCharQ.free(newChar);

  }
  */

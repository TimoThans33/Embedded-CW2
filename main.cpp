#include "mbed.h"
#include "SHA256.h"
#include "Thread.h"
#include "rtos.h"
#include "message.h"
#include "controller.h"
#include "decode.h"
#include "melody.h"





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



// Declarations

uint8_t HashCount = 0;


//**************************Function prototypes********************************
void computehash(void);
void counthash(void);
//*****************************************************************************


// Create a global instance of class Queue

Thread decodethread(osPriorityNormal,1024);
Thread messagethread(osPriorityNormal,1024);
//Thread melodythread(osPriorityNormal,1024);
Thread controllerThread(osPriorityNormal,1024);

Ticker t;

int main()
{
    setMail(START, 0);
    PWMPeriod(2000);
    ISRPhotoSensors();


    messagethread.start(getMail);
    decodethread.start(decode);
    controllerThread.start(motorCtrlFn);
    //melodythread.start(melodyFN);


    *nonce = 0;
    t.attach(&counthash, 1.0);
    while (true) {
      computehash();
    }
}



void counthash(void){
  setMail(SEC, HashCount);
  HashCount = 0;
}

void computehash(void){
    if (newKeyAdded){
      newKey_mutex.lock();
      *key = newKey;
      newKey_mutex.unlock();
      newKeyAdded = false;
    }
    SHA256::computeHash(hash2, sequence, 64);
    if ((hash2[0]==0) && (hash2[1]==0)) {
      setMail(NONCE, (uint64_t)(*nonce&0xFFFFFFFF));
    }
    HashCount += 1;
    *nonce+=1;

  }

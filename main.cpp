#include "mbed.h"
#include "SHA256.h"
#include <Thread.h>
#include "rtos.h"
//#include <message.h>

//Photointerrupter input pins
#define I1pin D3
#define I2pin D6
#define I3pin D5

//Incremental encoder input pins
#define CHApin   D12
#define CHBpin   D11

//Motor Drive output pins   //Mask in output byte
#define L1Lpin D1           //0x01
#define L1Hpin A3           //0x02
#define L2Lpin D0           //0x04
#define L2Hpin A6          //0x08
#define L3Lpin D10           //0x10
#define L3Hpin D2          //0x20

#define PWMpin D9

//Motor current sense
#define MCSPpin   A1
#define MCSNpin   A0

//Test outputs
#define TP0pin D4
#define TP1pin D13
#define TP2pin A2

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
//Drive state to output table
const int8_t driveTable[] = {0x12,0x18,0x09,0x21,0x24,0x06,0x00,0x00};

//Mapping from interrupter inputs to sequential rotor states. 0x00 and 0x07 are not valid
const int8_t stateMap[] = {0x07,0x05,0x03,0x04,0x01,0x00,0x02,0x07};
//const int8_t stateMap[] = {0x07,0x01,0x03,0x02,0x05,0x00,0x04,0x07}; //Alternative if phase order of input or drive is reversed

//Phase lead to make motor spin
const int8_t lead = 2;  //2 for forwards, -2 for backwards

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

//Status LED
DigitalOut led1(LED1);

//Photointerrupter inputs
InterruptIn I1(I1pin);
InterruptIn I2(I2pin);
InterruptIn I3(I3pin);

//Motor Drive outputs
DigitalOut L1L(L1Lpin);
DigitalOut L1H(L1Hpin);
DigitalOut L2L(L2Lpin);
DigitalOut L2H(L2Hpin);
DigitalOut L3L(L3Lpin);
DigitalOut L3H(L3Hpin);

DigitalOut TP1(TP1pin);
PwmOut MotorPWM(PWMpin);


// Declarations
volatile int _count;
int8_t orState = 0;
int8_t intState = 0;
int8_t intStateOld = 0;

//**************************Function prototypes********************************
void motorOut(void);
int8_t motorHome();
inline int8_t readRotorstate();
void drive(void);
void setOrState(int8_t state);
//void serialISR(void);
//void serial_queue(void);
//void computeHash(void);
//*****************************************************************************

Mutex key_mutex;
// Create a global instance of class Queue
//Queue<uint8_t, 8> inCharQ;

//Thread decodethread;
Thread messagethread;

int main()
{
    //setMail(START, 0);

    const int32_t PWM_PRD = 2500;
    MotorPWM.period_us(PWM_PRD);
    MotorPWM.pulsewidth_us(PWM_PRD);

    //messagethread.start(getMa);
    //decodethread.start(callback(serial_queue));
    //Initialise the serial port
    RawSerial pc(SERIAL_TX, SERIAL_RX);
    //Run the motor synchronisation
    orState = motorHome();
    setOrState(orState);
    //orState is subtracted from future rotor state inputs to align rotor and motor states
    //Poll the rotor state and set the motor outputs accordingly to spin the motor
    I1.rise(&drive);
    I2.rise(&drive);
    I3.rise(&drive);
    I1.fall(&drive);
    I2.fall(&drive);
    I3.fall(&drive);

    drive();

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




//Set a given drive state
void motorOut(int8_t driveState)
{

    //Lookup the output byte from the drive state.
    int8_t driveOut = driveTable[driveState & 0x07];

    //Turn off first
    if (~driveOut & 0x01) L1L = 0;
    if (~driveOut & 0x02) L1H = 1;
    if (~driveOut & 0x04) L2L = 0;
    if (~driveOut & 0x08) L2H = 1;
    if (~driveOut & 0x10) L3L = 0;
    if (~driveOut & 0x20) L3H = 1;

    //Then turn on
    if (driveOut & 0x01) L1L = 1;
    if (driveOut & 0x02) L1H = 0;
    if (driveOut & 0x04) L2L = 1;
    if (driveOut & 0x08) L2H = 0;
    if (driveOut & 0x10) L3L = 1;
    if (driveOut & 0x20) L3H = 0;
}

//Convert photointerrupter inputs to a rotor state
inline int8_t readRotorState()
{
    return stateMap[I1 + 2*I2 + 4*I3];
}

//Basic synchronisation routine
int8_t motorHome()
{
    //Put the motor in drive state 0 and wait for it to stabilise
    motorOut(0);
    ThisThread::sleep_for(2.0);

    //Get the rotor state
    return readRotorState();
}
void drive() {
    static int8_t intStateOld;
    intState = readRotorState();
    motorOut((intState-orState+lead+6)%6); //+6 to make sure the remainder is positive
    intStateOld = intState;
}

void setOrState(int8_t state){
    orState = state;
}





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

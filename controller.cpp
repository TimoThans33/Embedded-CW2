#include "controller.h"



//Drive state to output table
const int8_t driveTable[] = {0x12,0x18,0x09,0x21,0x24,0x06,0x00,0x00};

//Mapping from interrupter inputs to sequential rotor states. 0x00 and 0x07 are not valid
const int8_t stateMap[] = {0x07,0x05,0x03,0x04,0x01,0x00,0x02,0x07};

//Phase lead to make motor spin
const int8_t lead = 2;  //2 for forwards, -2 for backwards


int8_t orState = 0;

//Status LED
DigitalOut led1(LED1);

//Photointerrupter inputs
InterruptIn I1(I1pin);
InterruptIn I2(I2pin);
InterruptIn I3(I3pin);

//Motor Drive outputs
PwmOut L1L(L1Lpin);
DigitalOut L1H(L1Hpin);
PwmOut L2L(L2Lpin);
DigitalOut L2H(L2Hpin);
PwmOut L3L(L3Lpin);
DigitalOut L3H(L3Hpin);

PwmOut PWMCtrl(PWMpin);

uint32_t motorPWM=1000;


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

void ISRPhotoSensors() {
  I1.rise(&driveISR);
  I2.rise(&driveISR);
  I3.rise(&driveISR);
  I1.fall(&driveISR);
  I2.fall(&driveISR);
  I3.fall(&driveISR);
}

void PWMPeriod(int period) {
  L1L.period_us(period);
  L2L.period_us(period);
  L3L.period_us(period);
}

//Basic synchronisation routine
void motorHome(){
    //Put the motor in drive state 0 and wait for it to stabilise
    motorOut(0);
    ThisThread::sleep_for(2.0);
    orState = readRotorState();
}

void driveISR() {
    static int8_t oldState = 0;
    int8_t currentState = readRotorState();
    motorOut((currentState-orState+lead+6)%6); //+6 to make sure the remainder is positive
    oldState = currentState;
}



void driveCtrl() {

  // Set home position
  motorHome();




  while (1) {

  }
}

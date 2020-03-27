#include "controller.h"



//Drive state to output table
const int8_t driveTable[] = {0x12,0x18,0x09,0x21,0x24,0x06,0x00,0x00};

//Mapping from interrupter inputs to sequential rotor states. 0x00 and 0x07 are not valid
const int8_t stateMap[] = {0x07,0x05,0x03,0x04,0x01,0x00,0x02,0x07};

//Phase lead to make motor spin
volatile int8_t lead = 2;  //2 for forwards, -2 for backwards


volatile int32_t rotorPosition = 0;
float diffVelError = 0;
float intVelError = 0;
float diffRotError = 0;
float intRotError = 0;
float oldVelError = 0;
float oldRotError = 0;

int32_t rotation = 0;
uint32_t orState = 0;

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

//Define frequencies of some notes
#define C3 130
#define D3 146
#define E3 164
#define F3 174
#define G3 196
#define A3 220
#define B3 246

PwmOut PWMRotorControl(PWMpin);

uint32_t motorPWM = 0;
bool rotateTrue = false;

volatile float velPWM = 0;
volatile float rotPWM = 0;
volatile float vel = 0;
volatile float rot = 0;



//Set a given drive state
void motorOut(int8_t driveState, uint32_t torque)
{
    led1 = !led1;
    //Lookup the output byte from the drive state.
    int8_t driveOut = driveTable[driveState & 0x07];
    PWMRotorControl.pulsewidth_us(torque);

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

void PWMPeriod(int32_t period) {
  PWMRotorControl.period_us(period);
}


//Basic synchronisation routine
void motorHome(){
    //Put the motor in drive state 0 and wait for it to stabilise
    motorOut(0,PWM_LIMIT);
    ThisThread::sleep_for(2.0);
    orState = readRotorState();
}

void driveISR() {

    static int8_t oldState = 0;
    int8_t currentState = readRotorState();
    int8_t outState = (currentState-orState+lead+6)%6;
    motorOut(outState,motorPWM);

    // Calculate rotor position change
    if (oldState - currentState == 5) rotorPosition ++;
    else if (currentState - oldState == 5) rotorPosition --;
    else rotorPosition += currentState - oldState;
    oldState = currentState;
}

void motorCtrlTick(){
  controllerThread.signal_set(0x1);
}


uint32_t velocityController(){
  // y_s = k_p(s-v)
  float y_s;

  float velError = abs(velTarget)-abs(vel);

  // For reverse
  if (velTarget<0) lead = -2;
  else lead = 2;

  intVelError += velError;
  if (intVelError > VEL_DIFF_MAX) intVelError =  VEL_DIFF_MAX;
  if (intVelError < -VEL_DIFF_MAX) intVelError = -VEL_DIFF_MAX;

  y_s = VEL_CONST*velError + VEL_INT_CONST*intVelError;


  if (y_s > PWM_LIMIT) y_s = PWM_LIMIT;
  else if (y_s < 0) y_s = 0;

  return (uint32_t)y_s;
}

uint32_t positionController(){
  // y_r = k_p *E_r + k_d dE_r/dt
  float y_r;

  float rotError = rotTarget-rot;

  float diffRotError = (rotError - oldRotError);

  oldRotError = rotError;

  intRotError += rotError;
  if (intRotError >  POS_DIFF_MAX) intRotError =  POS_DIFF_MAX;
  if (intRotError < -POS_DIFF_MAX) intRotError = -POS_DIFF_MAX;

  y_r = POS_CONST*rotError + POS_DIFF_CONST*diffRotError + POS_INT_CONST*intRotError;

  // For reverse
  if (y_r<0) lead = -2;
  else lead = 2;

  y_r = abs(y_r);
  if (y_r > PWM_LIMIT2) y_r = PWM_LIMIT2;

  return (uint32_t)y_r;

}


void motorCtrlFn() {


  // Set home position
  motorHome();

  Ticker motorCtrlTicker;
  motorCtrlTicker.attach_us(&motorCtrlTick,100000);


  int8_t counter = 0;
  float oldRotorPosition = 0.0;
  int8_t oldState = 0;

  while (1) {

    controllerThread.signal_wait(0x1);

    vel = 10*((float)rotorPosition/6 - oldRotorPosition);
    oldRotorPosition = (float)rotorPosition/6;
    // Full rotations
    rot = rotorPosition/6;


    if (abs(vel) < 1 && velTarget != 0 && rotTarget-rot != 0) {
      // Starting the motor if not moving
      int8_t currentState = readRotorState();
      motorOut((currentState-orState+lead+6)%6,motorPWM);

    }
    if (velTarget-vel != 0) {
      // Set motor PWM
      velPWM = velocityController();
      rotPWM = positionController();
      if (vel < 0) motorPWM = (velPWM > rotPWM) ? velPWM : rotPWM;
      else motorPWM = (velPWM < rotPWM) ? velPWM : rotPWM;

    }

    if (rotTarget-rot != 0 && velTarget != 0 && counter == 10) {

      setMail(MOTOR, motorPWM);
      setMail(VELOCITY,vel);
      setMail(ROTOR, rot);
      setMail(ERROR,lead);


    }
    if (counter == 10){
      counter = 0;
    } else{
      counter += 1;
    }
  }
}

#include "message.h"
// Mail handles all incoming messages
Mail<message_struct, 16> outMail;

// Enable writing to the serial port
RawSerial pc(SERIAL_TX, SERIAL_RX);

// Function to set the incoming data and code into the mail queue
void setMail(uint8_t command, int32_t data){
    message_struct *mail = outMail.alloc();
    mail->data = data;
    mail->command = command;
    outMail.put(mail);
}

// Function which checks the mail queue after new commands
void getMail(){
  while (1) {
      osEvent newEvent = outMail.get();
      message_struct *mail = (message_struct*)newEvent.value.p;
      switch (mail->command) {
        case (START):
            pc.printf("Start %d, %d, \r\n", mail->command, mail->data);
            break;
        case (MOTOR):
            pc.printf("Motor power %d\r\n",mail->data);
            break;
        case (VELOCITY):
            pc.printf("Velocity of motor %d\r\n", mail->data);
            break;
        case (SET_VELOCITY):
            pc.printf("Target velocity of motor %d\r\n", mail->data);
            break;
        case (SET_ROTATION):
            pc.printf("Target rotation: %d\r\n", mail->data);
            break;
        case (ROTOR):
            pc.printf("Position of rotor %d\r\n", mail->data);
            break;
        case (HIT):
            pc.printf("Nonce: %d  \r\n", mail->data);
            break;
        case (SEC):
            pc.printf("Hash count: %d   \r\n", mail->data);
            break;
        case (ERROR):
            pc.printf("Error code: %d   \r\n", mail->data);
            break;
        case (KEY_UPPER):
            pc.printf("Key is: %x\r\n", mail->data);
        case (TONE):
            pc.printf("Music tone is: %d\r\n", mail->data);

      }
      outMail.free(mail);
    }
}

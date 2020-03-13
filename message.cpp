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
        case (MOTOR):
            pc.printf("Motor Position %d\r\n",mail->data);
        case (VELOCITY):
            pc.printf("Velocity of motor %d\r\n", mail->data);
        case (SET_VELOCITY):
            pc.printf("Target velocity of motor %d\r\n", mail->data);

      }
      outMail.free(mail);
    }
}

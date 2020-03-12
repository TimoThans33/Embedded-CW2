
// Mail handles all incoming messages
Mail<message_t, 16> outMail;

// Enable writing to the serial port
RawSerial pc(SERIAL_TX, SERIAL_RX);

// Function to set the incoming data and code into the mail queue
void setMail(uint32_t data, uint8_t command){
    message_t *mail = outMail.alloc();
    mail->data = data;
    mail->command = command;
    mail_box.put(mail);
}

// Function which checks the mail queue after new commands
void getMail(){
  while (1) {
      osEvent newEvent = outMail.get();
      message_t *mail = (message_t*)newEvent.value.p;
      switch (mail->command) {
        case (START):
            pc.printf("Start %d, %d, \r\n", mail->command, mail->data);
      }
      outMail.free(mail);
    }
}

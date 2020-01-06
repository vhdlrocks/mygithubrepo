/*
*
* Cellular Modem .ino module
*  
* FUNCTION CALL LIST
* void initCellModem(void)
* void setupRxSMS(void)
* int SendSMS(String RecipientNumber, String Message)
* int checkForUrSMS(void)
* void checkForFreshSMS(void)
* int PrintModemResponse(void)
* int DitchModemResponse(void)
* void cellModemOff(void)
*
*/

void initCellModem(void)
{
  // Initialize serial port to communicate with modem
  Serial.println("Initializing modem COM port");
  Serial1.begin(115200);
  delay(1000);
  while (!Serial1);

  // send command to modem to get firmware version
  Serial1.println("AT+CGMR");
  while(!serial1WaitFor("OK"));
  
  delay(10);

  // get modem response and print to debug serial on PC via USB port
  //while (PrintModemResponse() > 0);

  Serial.println("Waiting for network connection");
  while (!connectionGood) {

    // send command to modem to get network status
    Serial1.println("AT+CREG?");
    currentString = "";
    delay(500);

    // Read serial port buffer for UART connected to modem and print that message back out to debug serial over USB
    while (Serial1.available() > 0) {
      //read incoming byte from modem
      incomingByte = Serial1.read();
      //write byte out to debug serial over USB
      Serial.write(incomingByte);

      // add current byte to the string we are building
      currentString += char(incomingByte);

      // check currentString to see if network status is "0,1" or "0,5" which means we are connected
      if ((currentString.substring(currentString.length() - 3, currentString.length()) == "0,1") or
          (currentString.substring(0, currentString.length()) == "0,5")) {
        connectionGood = true;
        Serial.println();
        while(!serial1WaitFor("OK"));
        Serial.println("Connection successful!");
        
      }
    }
  }
  // get modem response, if any
  //while (PrintModemResponse() > 0);

  // put modem into text mode
  Serial1.println("AT+CMGF=1");
  while(!serial1WaitFor("OK"));
  //delay(250);
  //while (PrintModemResponse() > 0);
}


/*******************************************************************************************
*
* setupRxSMS
*
*******************************************************************************************/
void setupRxSMS(void)
{
  // read sms using the last index received
  Serial.println("Setting SMS polling interrupt scheme");
  //Serial1.print("AT+CNMI=2"); // send the message when its recieved
  Serial1.print("AT+CNMI=0"); // Don't do anything when recieved, I will poll for it
  Serial1.print("\r");
}

/*******************************************************************************************
*
* SendSMS
*
*******************************************************************************************/
int SendSMS(String RecipientNumber, String Message) 
{
  // assemble SMS message and send
  Serial1.print("AT+CMGS=\"+");
  Serial1.print(RecipientNumber);
  Serial1.print("\"\r");
  delay(250);
  while (PrintModemResponse() > 0);

  // check SMS message length and trim to 160bytes if necessary
  if (Message.length() > 160) {
    Serial.println("SMS too long, cropping to 160 bytes");
    Message = Message.substring(0, 160);
  }
  Serial1.print(Message);
  Serial1.write(26); // special "CTRL-Z" character
  Serial1.write("\r");
  delay(1000);
  while (PrintModemResponse() > 0);

  // let user know message has been sent
  Serial.println("SMS sent");

  // wait a few seconds so we don't spam the recipient
  delay(2000);
  return 1;
}

/*******************************************************************************************
*
* checkForUrSMS
*
*******************************************************************************************/
int checkForUrSMS(void) 
{
  // get unread messages
  Serial1.print("AT+CMGL=\"REC UNREAD\"");
  Serial1.print("\r");
  //serial1WaitFor("OK");
  delay(250);

  while (Serial1.available() > 0)
  { // Don't read unless
    RESPONSE +=  char(Serial1.read()); // Read a character
  }
  delay(250);

  // don't print blank lines
  if (RESPONSE != "")
  {
    Serial.println();
    Serial.println(RESPONSE);
  }

  if (RESPONSE.indexOf("+CMGL:") < 0) {
    // modem is telling us there are no unread messages so wait longer...
    Serial.println("No UNREAD messages");
  }
  else
  {
    // we received a message, so send something back
    Serial.println("UNREAD message(s) received");

    // parse out the sender's phonen number
    // find the "," portion of the message, the phone number is the next 10 digits
    startlocation = RESPONSE.indexOf("\",\"");

    // add leading character
    ResponseNumber = "1";

    // parse out the phone number
    for (int i = 3; i < 13; i++)
    {
      ResponseNumber += RESPONSE.charAt(startlocation + i);
    }

    // re-package this string to be parsed by function
    ResponseNumber += String("\",\"");
    Serial.println(ResponseNumber);

    // check for SMS cmd on/off condition
    if (RESPONSE.indexOf(flashToString(SN_ON_CMD_addr, SN_ON_CMD_length)) >= 0)
    {
      contactAction(1);
      cmdTurnOnOff(ResponseNumber, 1);
    }
    // check for SMS cmd on/off condition
    if (RESPONSE.indexOf(flashToString(SN_OFF_CMD_addr, SN_OFF_CMD_length)) >= 0)
    {
      contactAction(0);
      cmdTurnOnOff(ResponseNumber, 0);
    }

    // send a fun SMS in reseponse
    //SendSMS(ResponseNumber, String("Temperature sensor reading is ") += String (GetTemp()) += String("\r"));
    ResponseNumber = "";
  }
  RESPONSE = "";

  // let user know message has been sent
  Serial.println("Completed READ SMS process");

  // wait a few seconds so we don't spam the recipient
  delay(200);
  delay(20000);
  return 1;
}

/*******************************************************************************************
*
* checkForFreshSMS
*
*******************************************************************************************/
void checkForFreshSMS(void)
{
  while (Serial1.available() > 0)
  { // Don't read unless
    RCVSMSMSG +=  char(Serial1.read()); // Read a character
  }

  // stop the println cmd on output
  if (RCVSMSMSG != "")
  {
    Serial.println();
    Serial.println(RCVSMSMSG);
    delay(250);
  }

  // check for SMS cmd on/off condition
  if (RCVSMSMSG.indexOf(flashToString(SN_ON_CMD_addr, SN_ON_CMD_length)) >= 0)
  {
    contactAction(1);
    cmdTurnOnOff(RCVSMSMSG, 1);
  }
  // check for SMS cmd on/off condition
  if (RCVSMSMSG.indexOf(flashToString(SN_OFF_CMD_addr, SN_OFF_CMD_length)) >= 0)
  {
    contactAction(0);
    cmdTurnOnOff(RCVSMSMSG, 0);
  }

  RCVSMSMSG = "";
}

/*******************************************************************************************
*
* PrintModemResponse
*
*******************************************************************************************/
int PrintModemResponse(void) 
{
  while (Serial1.available() > 0) {
    //read incoming byte from modem and write byte out to debug serial over USB
    Serial.write(Serial1.read());
  }
  //return number of characters in modem response buffer -- should be zero, but some may have come in since last test
  return Serial1.available();
}

/*******************************************************************************************
*
* DitchModemResponse
*
*******************************************************************************************/
int DitchModemResponse(void)
{
    while(Serial1.available() >0) {
    //read incoming byte from modem and write byte out to debug serial
    d = Serial1.read();
  } 
  //return number of characters in modem response buffer -- should be zero, but some may have come in since last test
  return Serial1.available();
}

/*******************************************************************************************
*
* cellModemOn
*
*******************************************************************************************/
void cellModemOn(void)
{
  // Start cellular modem
  Serial.println("Starting Cellular Modem");
  // Arduino I/O pin 12 is connected to modem ON_OFF signal.
  // ON_OFF has internal 200k pullup resister and needs to be driven low by external signal for >1s to startup
  // Arduino defaults to I/O pin as input with no pullup on reset
  // Set I/O pin 12 to low
  digitalWrite(12, LOW);

  // Configure I/O pin 12 as output
  pinMode(12, OUTPUT);

  // Drive I/O pin 12 low
  digitalWrite(12, LOW);
  delay(1100); // modem requires >1s pulse

  // Return I/O pin 12 to input/hi-Z state
  pinMode(12, INPUT);
    
  // Requires 10 seconds after reset
  delay(10000);
}

/*******************************************************************************************
*
* cellModemOff
*
*******************************************************************************************/
void cellModemOff(void)
{
  Serial.println("Resetting Cellular Modem");
  digitalWrite(12, LOW);

  // Configure I/O pin 12 as output
  pinMode(12, OUTPUT);

  // Drive I/O pin 12 low
  digitalWrite(12, LOW);
  delay(2100); // modem requires >1s pulse

  // Return I/O pin 12 to input/hi-Z state
  pinMode(12, INPUT);
  
  // Requires 10 seconds after reset
  delay(10000);
}

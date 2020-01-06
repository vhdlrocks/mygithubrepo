


/*******************************************************************************************
*
* cmdTurnOnOff
*
*******************************************************************************************/
void cmdTurnOnOff(String RCVSMSMSG, int state)
{
  // find the "," portion of the message, the phone number is the next 10 digits
  startlocation = RCVSMSMSG.indexOf("\",\"") - 13;

  // add leading character
  ResponseNumber = "1";

  // parse out the phone number
  for (int i = 3; i < 13; i++) {
    ResponseNumber += RCVSMSMSG.charAt(startlocation + i);
  }

  // send a fun SMS in reseponse
  if (state)
  {
    SendSMS(ResponseNumber, String("Unit ") += String ("is on ") += String("\r"));
  }
  else
  {
    SendSMS(ResponseNumber, String("Unit ") += String ("is off ") += String("\r"));
  }
  ResponseNumber = "";

  RCVSMSMSG = "";
}

void initContact(void)
{
  // setup LED
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
  
  // set contact output to what is stored in flash
  contactAction(EEPROM.read(contactFlashAddr));
}

void contactAction(int contactVal)
{
  if(contactVal)
  {
    EEPROM.write(contactFlashAddr, 1);
    digitalWrite(7, HIGH);
  }
  else
  {
    EEPROM.write(contactFlashAddr, 0);
    digitalWrite(7, LOW);
  }
}

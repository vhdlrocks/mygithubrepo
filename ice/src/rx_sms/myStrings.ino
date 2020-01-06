


int serial1WaitFor(String newStr)
{
  String tempStr = "";
  while (Serial1.available() > 0)
  { // Don't read unless
    delay(250);
    tempStr +=  char(Serial1.read()); // Read a character
//    delay(100);
//    tempStr +=  char(Serial1.read()); // Read a character
  }
  
  //Check for correct string
  if(tempStr.indexOf(newStr) >= 0)
  {
    Serial.println(tempStr);
    tempStr = "";
    return 1;
  }
  
  if(tempStr.indexOf("ERROR") >= 0)
  {
    Serial.println(tempStr);
    Serial.println("Found ERROR!!!");
    Serial.println("Resetting Cell Modem and Firmware");
    cellModemOff();
    tempStr = "";
    software_Reset();
    Serial.println("Should not ever make it here!!!!");    
  }
  
  
  
  return 0;
}


void flashToSerialPrint(int startAddress, int numBytes)
{
  //loop through EEProm
  int flashIndexStart;
  int flashIndexEnd = startAddress + numBytes;
  for(flashIndexStart = startAddress; flashIndexStart < flashIndexEnd; flashIndexStart++)
  {
    Serial1.print(char(EEPROM.read(flashIndexStart)));
  }
}

String flashToString(int startAddress, int numBytes)
{
  String tempStr = "";
  //loop through EEProm
  int flashIndexStart;
  int flashIndexEnd = startAddress + numBytes;
  for(flashIndexStart = startAddress; flashIndexStart < flashIndexEnd; flashIndexStart++)
  {
    tempStr += char(EEPROM.read(flashIndexStart));
  }
  return tempStr;
}



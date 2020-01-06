


void initGPS(void)
{
  // setup GPS portion of DE910
  
        Serial1.println("AT$GPSLOCK?");
        while(!serial1WaitFor("OK"));
//        delay(500);
//        while(PrintModemResponse() > 0);
        
        Serial1.println("AT$GPSLOCK=0");
        while(!serial1WaitFor("OK"));
//        delay(500);
//        while(PrintModemResponse() > 0);
      
        Serial1.println("AT$GPSAT=1");
        while(!serial1WaitFor("OK"));
//        delay(250);
//        while(PrintModemResponse() > 0);
        
        Serial1.println("AT$NMEA=1");
        while(!serial1WaitFor("OK"));
//        delay(500);
//        while(PrintModemResponse() > 0);
      
        Serial1.println("AT$LOCMODE=4");
        while(!serial1WaitFor("OK"));
//        delay(250);
//        while(PrintModemResponse() > 0);
        
        Serial1.println("AT$GPSNMUN=1,1,1,1,1,0,0");
        while(!serial1WaitFor("OK"));
//        delay(250);
//        while(PrintModemResponse() > 0);
        
        Serial1.println("AT$GPSP=1");
        while(!serial1WaitFor("OK"));
//        delay(500);
//        while(PrintModemResponse() > 0);
        
        // already done in initCellModem()
//        delay(1000);
//  
//        Serial1.println("AT+CREG?");
//        delay(500);
//        while(PrintModemResponse() > 0);
  
}

void getGPSLocAndSendToWeb(void)
{
  //while(DitchModemResponse() > 0);
  
  // wait for Arduino reset
  Serial1.println("AT$GPSACP");
  delay(250);
  //while(PrintModemResponse() > 0);
      currentString = "";
      int i;
      i = 0;
      while(Serial1.available() >0) {
          //read incoming byte from modem
          incomingByte=Serial1.read();
        
          // add current byte to the string we are building
          currentString += char(incomingByte);
    
          // find commas in the string to parse out values of interest
          if(currentString.substring(currentString.length()-1, currentString.length()) == ","){
            coords[i] = currentString;
            currentString = "";
            i++;
        
            if (i == 3){
              break;
            }
          }
      }
      Serial.println("Coordinates");     
      Serial.println(coords[0]);
      Serial.println(coords[1]);
      Serial.println(coords[2]);
      
      
      // Exosite expects the coordinates to come in the format: DDMM.MMMM_DDMM.MMMM, 
      // so we need to take the Telit format and convert to Exosite format
      NS = coords[1].substring(0, coords[1].length()-2);
      EW = coords[2].substring(0, coords[2].length()-2);
      
      // trailing N & S characters indicate polarity of coordinate, need to strip the trailing character and add polarity to front of string
      if (coords[1].substring(coords[1].length()-2, coords[1].length()-1) == "N"){
        NS = NS;
      }else{
        NS = "-" + NS;
      }
      if (coords[2].substring(coords[2].length()-2, coords[2].length()-1) == "E"){
        EW = EW;
      }else{
        EW = "-" + EW;
      }
      // concatenate the lattitude and longitude into a single string
      finalCoords = "GPSdata=" + NS + "_" + EW;
      
      
      while(DitchModemResponse() > 0);
      
      //Serial.println(NS);
      //Serial.println(EW);
      Serial.println(finalCoords);
      
      // create the HTTP POST string
      // user should modify to use their specific Exosite CIK value
      if (finalCoords.length() > 16){
          //ExositePayload = "AT#HTTPSND=1,0,\"/onep:v1/stack/alias\",29,0,\"X-Exosite-CIK: 65930c137a0eefc5433357447f9fbbce33a80795\"";
          ExositePayload = "AT#HTTPSND=1,0,\"/onep:v1/stack/alias\",29,0,\"X-Exosite-CIK: ";
          Serial1.print(ExositePayload);

          flashToSerialPrint(CIK_EEPROM_ADDR, CIK_LENGTH);
          ExositePayload = "\"";
          // send the string to the modem to issue the HTTP POST to Exosite
          Serial1.println(ExositePayload);
          delay(250);
          while(PrintModemResponse() > 0);//while(DitchModemResponse() > 0);
          delay(250);
          while(PrintModemResponse() > 0);//while(DitchModemResponse() > 0);
          delay(1500);
          Serial1.println(finalCoords);
          delay(1000);
      } 
      delay(1000);
      while(DitchModemResponse() > 0);    
          
}

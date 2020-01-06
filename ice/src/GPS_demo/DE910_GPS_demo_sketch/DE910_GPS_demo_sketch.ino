/*
Skywire GPS tracker demo sketch 0.9
This sketch is a demonstration that enables the Skywire modem GPS capability using the Telit DE910 and sends location data to Exosite.

Intended Hardware:
* Arduino Leonardo
* Skywire Development kit or Arduino shield: NL-SWDK-EVDO-V or NL-SWAK
* Skywire Module: NL-SW-EVDO-V

created 1/25/2014
by Kurt Larson // NimbeLink.com

This example is provided as reference for use on Skywire devices only.
*/

// you must modify this line in order to send data to your Exosite account


// initialize a few variables
int incomingByte = 0;
int i = 0;
String coords[] = {"", "", ""};
boolean connectionGood = false;
String currentString = "";
String NS = "";
String EW = "";
String finalCoords = "";
int d;
String ExositePayload ="";

// code that initializes the serial ports, modem, and GPS peripheral
void setup() 
{
  // setup LEDs for demo
  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);
  digitalWrite(7, LOW);
  digitalWrite(6, LOW);

  // initialize serial debug communication with PC over USB connection
  Serial.begin(115200);
//  while (!Serial) ; // wait for serial debug port to connect to PC
  for (int q = 5; q > 0; q--){ 
 //   Serial.println(q, DEC);
    delay(1000);
  }
  Serial.println("GPS tracker Example");

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
  
  // wait for modem firmware to boot up, Telit datasheet indicates could take up to 10 seconds...
  Serial.println("Waiting 10 seconds for modem firmware to complete internal initialization.");
  for (int q = 10; q > 0; q--){ 
    Serial.println(q, DEC);
    delay(1000);
  } 
   
  // Initialize serial port to communicate with modem
  Serial.println("Initializing modem COM port");
  Serial1.begin(115200);
  while (!Serial1) ;

  delay(5000); 

  // get modem response, if any
  while(PrintModemResponse() > 0);
  
  // Test network connection
        Serial1.println("AT+CREG?");
        delay(500);
        while(PrintModemResponse() > 0);


  // Check modem firmware version, should be 15.00.024 as of this writing
  
          Serial1.println("AT+CGMR");
        delay(500);
        while(PrintModemResponse() > 0);
  
  
  // setup GPS portion of DE910
  
        Serial1.println("AT$GPSLOCK?");
        delay(500);
        while(PrintModemResponse() > 0);
        
        Serial1.println("AT$GPSLOCK=0");
        delay(500);
        while(PrintModemResponse() > 0);
      
        Serial1.println("AT$GPSAT=1");
        delay(250);
        while(PrintModemResponse() > 0);
        
        Serial1.println("AT$NMEA=1");
        delay(500);
        while(PrintModemResponse() > 0);
      
        Serial1.println("AT$LOCMODE=4");
        delay(250);
        while(PrintModemResponse() > 0);
        
        Serial1.println("AT$GPSNMUN=1,1,1,1,1,0,0");
        delay(250);
        while(PrintModemResponse() > 0);
        
        Serial1.println("AT$GPSP=1");
        delay(500);
        while(PrintModemResponse() > 0);
        
        delay(1000);
  
          Serial1.println("AT+CREG?");
        delay(500);
        while(PrintModemResponse() > 0);
  

  // setup TCP/IP connection
  
        // get IP address
        Serial1.println("at#sgact=1,1");
        delay(500);
        while(PrintModemResponse() > 0); 
        delay(5000);
        while(PrintModemResponse() > 0);  
        
        // configure Exosite HTTP POST server configuration
      Serial1.println("AT#HTTPCFG=1,\"m2.exosite.com\",80,0");
        delay(250);
        while(PrintModemResponse() > 0);  
        
 
     
        // wait 32 seconds before trying to get GPS signals
       Serial.println("Waiting 32 seconds for GPS to locate us.");
          for (int q = 32; q > 0; q--){ 
          Serial.print(q, DEC);
          Serial.print("..");
          delay(1000);
          } 
        Serial.println("");
  
      while(PrintModemResponse() > 0);
}


// main loop that sends the SMS message
void loop() 
{
while(1){
 delay(10000);
      
  while(DitchModemResponse() > 0);
  
  // wait for Arduino reset
  Serial1.println("AT$GPSACP");
  delay(250);
  //while(PrintModemResponse() > 0);
      currentString = "";
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
          ExositePayload = "AT#HTTPSND=1,0,\"/onep:v1/stack/alias\",29,0,\"X-Exosite-CIK: 65930c137a0eefc5433357447f9fbbce33a80795\"";
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
}

int PrintModemResponse(){
    while(Serial1.available() >0) {
    //read incoming byte from modem and write byte out to debug serial
    Serial.write(Serial1.read());
  } 
  //return number of characters in modem response buffer -- should be zero, but some may have come in since last test
  return Serial1.available();
}

int DitchModemResponse(){
    while(Serial1.available() >0) {
    //read incoming byte from modem and write byte out to debug serial
    d = Serial1.read();
  } 
  //return number of characters in modem response buffer -- should be zero, but some may have come in since last test
  return Serial1.available();
}


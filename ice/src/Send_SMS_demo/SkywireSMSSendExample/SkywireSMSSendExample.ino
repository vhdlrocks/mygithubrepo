/*
SMS send demo version 0.9
This sketch is an example that sends SMS messages using the Telit CE910-DUAL modem.

Circuit:
* Arduino Leonardo
* Skywire development kit: NL-SWDK-1xRTT and NL-SW-1xRTT

created 9/27/2013
by Kurt Larson // NimbeLink.com

This example is in the public domain.
*/

// assign destination phone number
// destination phone number of 1-555-444-3333 should be formated like: 15554443333
#define DESTINATION_PHONE_NUMBER "12083017145" //Jeff
//   #define DESTINATION_PHONE_NUMBER "17138184532" //Andy

// SMS message you want to send, limited to 160 characters
String SMS_MESSAGE = "Andy, this is the funky message from the module :)";

// initialize a few variables
int incomingByte = 0;
boolean connectionGood = false;
String currentString = "";

// code that initializes the serial ports and modem, waits for valid network connection
void setup() 
{
  // initialize serial debug communication with PC over USB connection
  Serial.begin(9600);
  while (!Serial) ; // wait for serial debug port to connect to PC
  for (int q = 5; q > 0; q--){ 
    Serial.println(q, DEC);
    delay(250);
  }
  Serial.println("SMS Send Example");

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
  
  // Initialize serial port to communicate with modem
  Serial.println("Initializing modem COM port");
  Serial1.begin(115200);
  while (!Serial1) ;
  
  // send command to modem to get firmware version
  Serial1.println("AT+CGMR");
  delay(10);
  
  // get modem response and print to debug serial on PC via USB port
  while(PrintModemResponse() > 0);

  Serial.println("Waiting for network connection"); 
  while(!connectionGood){
    
    // send command to modem to get network status
    Serial1.println("AT+CREG?");
    currentString = "";
    delay(500);
    
    // Read serial port buffer for UART connected to modem and print that message back out to debug serial over USB
    while(Serial1.available() >0) {
      //read incoming byte from modem
      incomingByte=Serial1.read();
      //write byte out to debug serial over USB
      Serial.write(incomingByte);
    
      // add current byte to the string we are building
      currentString += char(incomingByte);
    
      // check currentString to see if network status is "0,1" or "0,5" which means we are connected
      if((currentString.substring(currentString.length()-3, currentString.length()) == "0,1") or 
         (currentString.substring(0, currentString.length()) == "0,5")){
      connectionGood = true;
      Serial.println(); 
      Serial.println("Connection successful!");  
      }
  }
  }
  // get modem response, if any
  while(PrintModemResponse() > 0);
}


// main loop that sends the SMS message
void loop() 
{
  // put modem into text mode
  Serial1.println("AT+CMGF=1");
  delay(250);
  while(PrintModemResponse() > 0);
 
  // assemble SMS message and send
  Serial1.print("AT+CMGS=\"+");
  Serial1.print(DESTINATION_PHONE_NUMBER);
  Serial1.print("\"\r");
  delay(250);
  while(PrintModemResponse() > 0);
  
  // check SMS message length and trim to 160bytes if necessary
  if (SMS_MESSAGE.length() > 160){
    Serial.println("SMS too long, cropping to 160 bytes");
    SMS_MESSAGE = SMS_MESSAGE.substring(0, 160);
  }
  Serial1.print(SMS_MESSAGE);
  Serial1.write(26); // special "CTRL-Z" character
  Serial1.write("\r");
  delay(1000);
  while(PrintModemResponse() > 0);
  
  // let user know message has been sent
  Serial.println("SMS sent");

  // wait for Arduino reset
  while(1);
}

int PrintModemResponse(){
    while(Serial1.available() >0) {
    //read incoming byte from modem and write byte out to debug serial over USB
    Serial.write(Serial1.read());
  } 
  //return number of characters in modem response buffer -- should be zero, but some may have come in since last test
  return Serial1.available();
}


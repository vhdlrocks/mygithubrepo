/*
Easy Button Example version 0.9
This sketch is an example that sends SMS messages using the Telit CE910-DUAL modem when an external button is pressed.
If the device receives an SMS message, it replies to the sender with a message that includes some values read from the internal ADC.

Circuit:
* Arduino Leonardo
* Skywire development kit: NL-SWDK-1xRTT and NL-SW-1xRTT

created 9/27/2013
by Kurt Larson // NimbeLink.com

This example is in the public domain.
*/

// assign destination phone number
// destination phone number of 1-555-444-3333 should be formated like: 15554443333
#define DESTINATION_PHONE_NUMBER "15554443333"

// SMS message you want to send, limited to 160 characters
String SMS_MESSAGE = "Easy Button SMS Received. Get back to the booth!";

String RESPONSE = "";
String RCVSMSMSG = "";
String ResponseNumber = "";
// Easy button attaches to D2 and D3, there is a 5K pull down on D3
// D2 needs to be driven high, D3 needs to be input with pull ups disabled
// When Easy button is pressed, it creates a 1K resistance across D2, D3


//initialize a few variables
int incomingByte = 0;
boolean connectionGood = false;
String currentString = "";
String strInt = "";

// setup variables for reading incoming SMS messages
int startlocation = 0;
int endlocation = 0;

/*******************************************************************************************
*
* SETUP
*
*******************************************************************************************/
// code that initializes the serial ports and modem, waits for valid network connection
void setup() 
{
  // setup D2 as output driving low
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  
  // setup D3 as input 
  pinMode(3, INPUT);

// setup internal temp sensor stuff
  ADMUX = 1<<REFS1 | 1<<REFS0 | 0x47; //2.56V reference, set temp. sensor to input to ADC
  ADCSRA = 1<<ADEN | 1<<ADSC | 0x07;  //enable ADC, start conversion, 2MHz clock

  // initialize serial debug communication with PC over USB connection
  Serial.begin(9600);
//  while (!Serial) ; // wait for serial debug port to connect to PC
//  for (int q = 5; q > 0; q--){ 
//    Serial.println(q, DEC);
//    delay(250);
//  }
  delay(3000);
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
  
  // put modem into text mode
  Serial1.println("AT+CMGF=1");
  delay(250);
  while(PrintModemResponse() > 0);

}

/*******************************************************************************************
*
* MAIN LOOP
*
*******************************************************************************************/
void loop() 
{
   bitSet(ADCSRA, ADSC); // start next conversion
  // put modem into text mode
  Serial.println("Waiting for new SMS");
  //delay(250);
  //Serial.println(GetTemp(),1);
  //delay(250);
 
  // read D3 to see if button has been pressed, if pressed, send an SMS!  
//  while(digitalRead(3) == 1){
//      SendSMS(DESTINATION_PHONE_NUMBER, SMS_MESSAGE);  
//   }
//   
   ReadSMS();
}

/*******************************************************************************************
*
* PrintModemResponse
*
*******************************************************************************************/
int PrintModemResponse(){
    while(Serial1.available() >0) {
    //read incoming byte from modem and write byte out to debug serial over USB
    Serial.write(Serial1.read());
  } 
  //return number of characters in modem response buffer -- should be zero, but some may have come in since last test
  return Serial1.available();
}


/*******************************************************************************************
*
* SendSMS
*
*******************************************************************************************/
int SendSMS(String RecipientNumber, String Message){
  // assemble SMS message and send
  Serial1.print("AT+CMGS=\"+");
  Serial1.print(RecipientNumber);
  Serial1.print("\"\r");
  delay(250);
  while(PrintModemResponse() > 0);
  
  // check SMS message length and trim to 160bytes if necessary
  if (Message.length() > 160){
    Serial.println("SMS too long, cropping to 160 bytes");
    Message = Message.substring(0, 160);
  }
  Serial1.print(Message);
  Serial1.write(26); // special "CTRL-Z" character
  Serial1.write("\r");
  delay(1000);
  while(PrintModemResponse() > 0);
  
  // let user know message has been sent
  Serial.println("SMS sent");
  
  // wait a few seconds so we don't spam the recipient
  delay(2000);
  return 1;  
}

/*******************************************************************************************
*
* ReadSMS
*
*******************************************************************************************/
int ReadSMS(void){
 // get unread messages
  Serial1.print("AT+CMGL=\"REC UNREAD\"");
  //Serial1.print("AT+CMGR");
  Serial1.print("\r");
  delay(250);
  
  while (Serial1.available() > 0){ // Don't read unless
         //   char1 = Serial1.read(); // Read a character
         //   char2 = Serial1.read(); // read 2nd character
            RESPONSE +=  char(Serial1.read()); // Read a character
  }
  delay(250);
  //while(PrintModemResponse() > 0); 
  Serial.println();
  Serial.println(RESPONSE);
 
  
  if(RESPONSE.indexOf("OK") >= 0){
    // modem is telling us there are no unread messages so wait longer...
    Serial.println("No messages to read!");
  } 
  else
  {
     //*************************************** START SMS Recieve
//  // put modem into text mode
//  Serial1.println("AT+CSDH=1");
//  delay(250);
//  while(PrintModemResponse() > 0);
//
//  // get Nimbelink index of most recent SMS
//  startlocation = RESPONSE.indexOf(":") + 2;
//  endlocation = RESPONSE.indexOf(",");
//  int index;
//  index = strInt.toInt();
// 
//  // assign memr index value for reading
//  strInt = RESPONSE.substring(startlocation,endlocation);
//  
//  // read sms using the last index received
//  Serial1.print("AT+CMGR=");
//  Serial1.print(strInt);
//  Serial1.print("\r");
//  
//  // get and print the sms message
//  delay(250);
//  while (Serial1.available() > 0){ // Don't read unless
//            RCVSMSMSG +=  char(Serial1.read()); // Read a character
//  }
//  Serial.println();
//  Serial.println(RCVSMSMSG);
//  
//  delay(250);
//  while(PrintModemResponse() > 0);
//  Serial.println();
  //*************************************** End SMS Recieve
  
    // we received a message, so send something back
    Serial.println("Got a message, sending a response");
    // parse out the sender's phonen number
    
    // find the "," portion of the message, the phone number is the next 10 digits
    startlocation = RESPONSE.indexOf("\",\"");
    
    // add leading character
    ResponseNumber = "1"; 
    
    // parse out the phone number
    for (int i = 3; i <13; i++){
      ResponseNumber += RESPONSE.charAt(startlocation + i);
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
* GetTemp
*
*******************************************************************************************/
int GetTemp(void){
   return ((((ADC - 442)*9)/5)+32); 
   //return ADC; 
}



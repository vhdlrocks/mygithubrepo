#include <EEPROM.h>

/*
*
* MAIN
*
*/



// assign destination phone number
// destination phone number of 1-555-444-3333 should be formated like: 15554443333
#define DESTINATION_PHONE_NUMBER "15554443333"

// SMS message you want to send, limited to 160 characters
String SMS_MESSAGE = "This is an SMS test";

String RESPONSE = "";
String RCVSMSMSG = "";
String ResponseNumber = "";

int incomingByte = 0;
boolean connectionGood = false;
String currentString = "";
String strInt = "";

int swCntr = 0;

// setup variables for reading incoming SMS messages
int startlocation = 0;
int endlocation = 0;

//gps .ino file
String coords[] = {"", "", ""};
String NS = "";
String EW = "";
String finalCoords = "";
int d;
String ExositePayload ="";

int SN_ON_CMD_addr = 40;
int SN_ON_CMD_length = 9;
String SN_ON_CMD = "35X24C ON";

int SN_OFF_CMD_addr = 50;
int SN_OFF_CMD_length = 10;
String SN_OFF_CMD = "35X24C OFF";

// start address in EEProm of Exosite CIK
int CIK_EEPROM_ADDR = 0;
int CIK_LENGTH = 40;
String EXOSITE_CIK = "0123456789012345678901234567890123456789"; //"65930c137a0eefc5433357447f9fbbce33a80795";

// contactCtrl.ino
int contactFlashAddr = 60;
int ON_OFF_VAL = 1; //initialize to ON (1)

//myStrings .ino
String RSP = "";

int GPSON = 0;
/*******************************************************************************************
*
* SETUP
*
*******************************************************************************************/
// code that initializes the serial ports and modem, waits for valid network connection
void setup()
{

  
  initContact();
  initAnalog();

//***************** UNCOMMENT ME ONLY FOR DEBUG ****************************** 
  // initialize serial debug communication with PC over USB connection
  Serial.begin(9600);
//  while (!Serial) ; // wait for serial debug port to connect to PC
  for (int q = 5; q > 0; q--)
  {
    Serial.println(q, DEC);
    delay(250);
  }
//***************************************************************************/ 

  cellModemOff();
  cellModemOn();


  // Initialize Cellular Modem
  initCellModem();

  // Initialize GPS
  if(GPSON)initGPS();
  
  // Initialize TCP/IP connection
//  initTCPIP();
  
  // need to wait at least 32 seconds for GPS lock

  // Check for unread SMS
//  checkForUrSMS();

  // Setup SMS interrupts through Serial1 UART
//  setupRxSMS();
  
  Serial.println("System setup complete");
}

/*******************************************************************************************
*
* MAIN LOOP
*
*******************************************************************************************/
void loop()
{

  
//  if(Serial1.available() > 0)
//  {
//    checkForFreshSMS();
//  }

  
  // Need to use internal timer to update the GPS location

  swCntr++;
  delay(1);
//  if(swCntr == 300000)
  if(swCntr == 5000)
  {
    swCntr = 0;
  }
  
    if(swCntr == 1)
  {
    // Get GPS location and update web
    Serial.println();
    Serial.println();
//    Serial.println("Collecting GPS data");
//    getGPSLocAndSendToWeb();
    Serial.println("Getting Switch Data");
    exositeGet();
  }

//  if(swCntr == 2)
//  {
//    checkForUrSMS();
//  }
  
}

void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
  asm volatile ("  jmp 0");  
}  



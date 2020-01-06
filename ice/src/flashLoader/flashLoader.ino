#include <EEPROM.h>

// Length 40
int eeAddress = 0;   //Location we want the data to be put
String EXOSITE_CIK = "65930c137a0eefc5433357447f9fbbce33a80795";
String RTN_VAL = "";

int contactFlashAddr = 60;
int ON_OFF_VAL = 1; //initialize to ON (1)

int SN_ON_CMD_addr = 40;
int SN_ON_CMD_length = 9;
String SN_ON_CMD = "35X24C ON";

int SN_OFF_CMD_addr = 50;
int SN_OFF_CMD_length = 10;
String SN_OFF_CMD = "35X24C OFF";
int j;

void setup() {
  
  // Wait for serial port to open
  Serial.begin( 9600 );
  while (!Serial);
  
  // Erase the EEPROM
  for ( int i = 0 ; i < EEPROM.length() ; i++ )
  EEPROM.write(i, 0);
  
  Serial.println("EEPROM erased!");

  EEPROM.put( eeAddress, EXOSITE_CIK );

  // "ON" CMD
  //EEPROM.put( SN_ON_CMD_addr, SN_ON_CMD );
  j = 0;
  for(int i=SN_ON_CMD_addr;i<SN_ON_CMD_addr+SN_ON_CMD_length;i++)
  {
    EEPROM.write(i, SN_ON_CMD[j]);
    j++;
  }
  
  // "OFF" CMD
  j = 0;
  for(int i=SN_OFF_CMD_addr;i<SN_OFF_CMD_addr+SN_OFF_CMD_length;i++)
  {
    EEPROM.write(i, SN_OFF_CMD[j]);
    j++;
  }
  
  // "Contact default value" CMD
  j = 0;
  for(int i=contactFlashAddr;i<contactFlashAddr+1;i++)
  {
    EEPROM.write(i, ON_OFF_VAL);
    j++;
  }
  
  EEPROM.get( eeAddress, RTN_VAL );
  Serial.println(RTN_VAL);
  
  int k = 0;
  for(k=0;k<40;k++)
  {
    Serial.print(EEPROM.read(k));
  }
  Serial.println();
  
    for(k=0;k<40;k++)
  {
    EEPROM.write(k, EXOSITE_CIK[k]);
  }

  for(k=0;k<70;k++)
  {
    if(k==SN_ON_CMD_addr || k==SN_OFF_CMD_addr) Serial.println();
    Serial.print(char(EEPROM.read(k)));
    
  }
  Serial.println();
  
  Serial.println(EEPROM.read(contactFlashAddr));

}

void loop() {
  // put your main code here, to run repeatedly:

}

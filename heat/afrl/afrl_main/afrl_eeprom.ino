
void eepWrite() {

    int eeAddress = 0;   //Location we want the data to be put.

    // <SYSTEM VARIABLES>
    AFRLObject customVar = {
        CAM_A_ON,
        CAM_B_ON,
        RLY_0_ON  ,
        RLY_0_OFF  ,
        CAM_A_OFF ,
        CAM_B_OFF ,
        LED_RATE_SLOW  ,
        LED_RATE_FAST  
    };

    EEPROM.put(eeAddress, customVar);

}

void eepRead(bool debug) {
    int eeAddress = 0;

    AFRLObject customVar3; //Variable to store custom object read from EEPROM.
    EEPROM.get(eeAddress, customVar3);

    // <SYSTEM VARIABLES>
    if (debug) {
        Serial.println(customVar3.CAM_A_ON);
        Serial.println(customVar3.CAM_B_ON);
        Serial.println(customVar3.RLY_0_ON  );
        Serial.println(customVar3.RLY_0_OFF  );
        Serial.println(customVar3.CAM_A_OFF );
        Serial.println(customVar3.CAM_B_OFF );
        Serial.println(customVar3.LED_RATE_SLOW  );
        Serial.println(customVar3.LED_RATE_FAST  );
    }

    // <SYSTEM VARIABLES>
    CAM_A_ON = customVar3.CAM_A_ON;
    CAM_B_ON = customVar3.CAM_B_ON;
    RLY_0_ON   = customVar3.RLY_0_ON  ;
    RLY_0_OFF   = customVar3.RLY_0_OFF  ;
    CAM_A_OFF  = customVar3.CAM_A_OFF ;
    CAM_B_OFF  = customVar3.CAM_B_OFF ;
    LED_RATE_SLOW   = customVar3.LED_RATE_SLOW  ;
    LED_RATE_FAST   = customVar3.LED_RATE_FAST  ;

}


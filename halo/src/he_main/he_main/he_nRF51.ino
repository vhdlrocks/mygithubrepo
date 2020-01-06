
// To load a new Bluetooth broadcast name set LOAD_BT_NAME_TO_FLASH = 1
// and modify BT_BRAODCAST_NAME string definition
const int LOAD_BT_NAME_TO_FLASH = 0;
String BT_BRAODCAST_NAME = "HAL0-1 SN:100";


void init_nRF51(){

    /* Initialise the sensor */
    Serial.print("nRF51 BLE module status : ");

    if ( !ble.begin(VERBOSE_MODE) )
    {
        error(F("FAILURE"));
    }
    else{
        Serial.println( F("PASSED") );

        if ( FACTORYRESET_ENABLE )
        {
            /* Perform a factory reset to make sure everything is in a known state */
            Serial.println(F("Performing a factory reset: "));
            if ( ! ble.factoryReset() ){
            error(F("Couldn't factory reset"));
            }
        }

        /* Disable command echo from Bluefruit */
        ble.echo(false);

        if (LOAD_BT_NAME_TO_FLASH == 1){
            // String valToSend = "AT+GAPDEVNAME=";
            // valToSend += BT_BRAODCAST_NAME;
            // int length = valToSend.length() + 1;
            // char charBuf[length];
            // valToSend.toCharArray(charBuf, length);

            // ble.sendCommandCheckOK(valToSend);
            // ble.sendCommandCheckOK("AT+GAPDEVNAME=" BT_BRAODCAST_NAME);
            ble.sendCommandCheckOK("AT+GAPDEVNAME=HAL0-1 SN:100");
        }

        Serial.println("Requesting Bluefruit info:");
        /* Print Bluefruit information */
        ble.info();

        ble.verbose(false);  // debug info is a little annoying after this point!

        /* Wait for connection */
        Serial.print(F("Waiting for BLE connection: "));
        while (! ble.isConnected()) {
            digitalWrite(blueBaseLED, HIGH);
            delay(500);
            digitalWrite(blueBaseLED, LOW);
            delay(500);
        }

        Serial.println(F("CONNECTED"));

        for(int i = 0; i < 8; i++){
            digitalWrite(blueBaseLED, HIGH);
            delay(100);
            digitalWrite(blueBaseLED, LOW);
            delay(100);
        }

        // Blue base LED shall be on while connected
        digitalWrite(blueBaseLED, HIGH);

        Serial.println(F("******************************"));

        // LED Activity command is only supported from 0.6.6
        if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
        {
            // Change Mode LED Activity
            Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
            ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
        }

        // Set module to DATA mode
        Serial.println( F("...Switching to DATA mode") );
        ble.setMode(BLUEFRUIT_MODE_DATA);

        Serial.println(F("******************************"));

    }

}

void reconnect_nRF51(){

    /* Wait for connection */
    Serial.print(F("Waiting for BLE re-connection: "));
    while (! ble.isConnected()) {
        digitalWrite(blueBaseLED, HIGH);
        delay(500);
        digitalWrite(blueBaseLED, LOW);
        delay(500);
    }

    Serial.println(F("RE-CONNECTED"));

    for(int i = 0; i < 8; i++){
        digitalWrite(blueBaseLED, HIGH);
        delay(100);
        digitalWrite(blueBaseLED, LOW);
        delay(100);
    }

    // Blue base LED shall be on while connected
    digitalWrite(blueBaseLED, HIGH);

}

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

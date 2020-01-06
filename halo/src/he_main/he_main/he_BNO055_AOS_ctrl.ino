/*
    Project : HALO-1
    Author  : Jeff King
    Date    : 10/1/2018
*/

/* ----------------------------------------------
    init_aos : Initialize the AOS (Absolute Orientation Sensor)
-----------------------------------------------*/
// No references (global reference only: bno, Serial, delay)
void init_aos(){
    /* Initialise the sensor */
    Serial.print("BNO055 AOS sensor status: ");

    if(!bno.begin())
    {
        /* There was a problem detecting the BNO055 ... check your connections */
        Serial.println("FAILURE");
        while(1);
    }
    else{
        Serial.println("PASSED");
    }

    delay(1000);

    bno.setExtCrystalUse(true);
}
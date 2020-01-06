/*
    Project : HALO-1
    Author  : Jeff King
    Date    : 10/1/2018

    he_cmd  : Runs all valid commands from app

    Metrics from the testing on 12/20/2018:
    getAdcData        --> us: 10366
    convertToVolts    --> us: 471
    findPeak          --> us: 112
    CF_dataInit       --> us: 15
    getAngle          --> us: 236798
    analogRead        --> us: 426 (for the analogRead function only)
    bno.getEvent      --> us: 1000 - 2000

*/


/* ----------------------------------------------
    searchForMField :
-----------------------------------------------*/
// Gloabal              : vref, adcMaxTicks
// int numLoops         : number of times to look for an report an angle
// int showAllAngles    : 0 = Don't show angles on each loop
//                        1 = Show calculated angle on each loop
// int searchType       : Type of results to show
//                        0 = Only show last angle found
//                        1 = Only show averaged angle
//                        2 = Show averaged angle, standard deviation and range of angles found
// ----------------------------------------------
int searchForMField(int numLoops, int showAllAngles, int searchType) {

    float angleSum = 0.0;
    unsigned long startTime;
    unsigned long totalTime;
    float angleVals[numLoops];

    // Corrected angle based on position of hand WRT gravity
    float angleFinal;

    for(int i = 0; i < numLoops; i++){

        // Set initial conditions for low pass filter
        // setLPFilterInitCond();

        int m = 24;
        int peakType = 0;
        while(peakType == 0){

            // Exit loop if ble sends anything
            if (ble.available()){
                int c = 0;
                while(ble.available()){
                    c = ble.read();
                }
                return 1; // BLE interruption detected, end here
            }

            // Get a new sensor event
            // bno.getEvent(&event);

            // Get AOS Y-axis angle
            // aosAngleOffs = event.orientation.y;
            aosAngleOffs = 0.0;

            // read the analog in values
            getAdcData(0);

            // Convert the raw values to voltages and store them in adcVolt[all][3]
            convertToVolts(0);

            /* **********************************
                ADC Low pass filter
            ******************************** */
            // filterVoltsLP(1);

            /* **********************************
                Find max or min value
            ******************************** */
            m = findPeak();
            // peakType = getPeakType(m); // Indicates type of peak: none, max, min
            if (m < 0){ // Minima peak found
                m = -(m + 1);
                peakType = 2;
                digitalWrite(redBaseLED, HIGH); // Indicate field has been found
            }
            else{
                if (m == 24){ // No peak found
                    peakType = 0;
                }
                else{
                // else m >= 0 && m < 24, Maxima peak found
                    peakType = 1;
                    digitalWrite(redBaseLED, HIGH); // Indicate field has been found
                }
            }
            // Serial.println("Step1");
            if (peakType != 0){
                bno.getEvent(&event);

            }
            // Serial.println("Step2");

        }

        // Serial.println(peakType);
        /* **********************************
            Pre-process the data prior to the Curve fitting
        ******************************** */
        // Serial.println("Step3");
        int boundaryXed = CF_dataInit(m);

        // Normalize the magnitudes to keep numbers small
        // NOTE: This step may not be necessary due to sensor limits being 0V - 2V
        // normCF_Data();

        /* **********************************
            Run Curve fit (CF) algorithm and get angle
        ******************************** */
        // Serial.println("Step4");
        float maxCFVal = 0.0;
        if (peakType != 0){
            maxCFVal = getAngle(boundaryXed, peakType);
        }

        // We have the angle WRT the sensors, now correct
        // for the AOS position
        int y = int(event.orientation.y);
        int z = int(event.orientation.z);
        float y_float = int(event.orientation.y);
        float z_float = int(event.orientation.z);

        /* AOS angle correction WRT Gravity
        The 90 degree offset accounts for the AOS relationship to Gravity on the Y axis
        The -/+ of the 90 degree offset accounts for the AOS relationship to Gravity on the Z axis
        The + 15.0 degrees is due to the HE sensor (0) offset from the horizontal plane of the handle
        */
        // If thumb lever faces up
        if (abs(z) > 90) {
            aosAngleOffs = y_float - 90.0 + 15.0;
        }
        // Else thumb lever faces down (abs(z) > 90)
        else{
            aosAngleOffs = -y_float + 90.0 + 15.0;
        }

        // Get AOS Y-axis angle
        // aosAngleOffs = event.orientation.y;
        // totalTime = micros() - startTime;
        // Serial.print("Time in us: ");
        // Serial.println(totalTime);

        // Serial.print("AOS Angle x,y,z is: ");
        // Serial.print(event.orientation.x);
        // Serial.print(", ");
        // Serial.print(aosAngleOffs);
        // Serial.print(", ");
        // Serial.println(event.orientation.z);
                // aosAngleOffs = 0.0;

        // Get angle and start averaging
        // angleSum += maxCFVal + aosAngleOffs;
        // angleVals[i] = maxCFVal + aosAngleOffs;

        // Correct angle using the AOS position
        angleFinal = maxCFVal + aosAngleOffs;

        // Constrain values to always be within 0 - 360
        if (angleFinal < 0.0){
            angleFinal = angleFinal + 360.0;
        }
        else{
            if (angleFinal >= 360.0){
                angleFinal = angleFinal - 360.0;
            }
        }

        // Prepare a sum of angles for other calculations: Mean, Standard Deviation, etc.
        angleSum += angleFinal;
        // Capture values for Standard Deviation calculation
        if (searchType == 2) {
            angleVals[i] = angleFinal;
        }

        if (showAllAngles || searchType == 0) {
            String valToSend = "";
            valToSend += String(angleFinal);
            // valToSend += ",";
            // valToSend += m;
            if (i < numLoops){
                if ((searchType != 0) || (i != numLoops - 1)){
                    valToSend += ",";
                }
            }
            int length = valToSend.length() + 1;
            char charBuf[length];
            valToSend.toCharArray(charBuf, length);
            ble.println(charBuf);
        }
    }

    // Calculate angle mean
    // Send angle mean to BLE console if searchType == 1
    float angleMean;
    if (searchType != 0) {
        angleMean = (angleSum/float(numLoops));
        if (searchType == 1) {
            delay(200);
            String valToSend = "";
            valToSend += "AVG:";
            valToSend += String(angleMean);
            int length = valToSend.length() + 1;
            char charBuf[length];
            valToSend.toCharArray(charBuf, length);
            ble.println(charBuf);
        }
    }

    // Calculate angle standard deviation and range
    // Send to angle mean, standard deviation and range to BLE console if searchType == 2
    if (searchType == 2) {

        float angleStdDev;
        float angleValMinusMeanSqrd = 0.0;

        for(int i = 0; i < numLoops; i++){
            angleValMinusMeanSqrd += pow((angleVals[i] - angleMean), 2.0);
        }
        angleStdDev = sqrt(angleValMinusMeanSqrd/float(numLoops));

        // Get range from max to min

        // Get max
        float highNum = -400.0;
        for(int i = 0; i < numLoops; i++){
            if (angleVals[i] > highNum){
                highNum = angleVals[i];
            }
        }

        // Get min
        float lowNum = highNum;
        for(int i = 0; i < numLoops; i++){
            if (angleVals[i] < lowNum){
                lowNum = angleVals[i];
            }
        }

        float angleRange = highNum - lowNum;
        // Serial.print(highNum);
        // Serial.print(", ");
        // Serial.print(lowNum);
        // Serial.print(", ");
        // Serial.println(angleRange);


        // startTime = micros();
        // totalTime = micros() - startTime;
        // Serial.print("Time in us: ");
        // Serial.println(totalTime);
        delay(200);
        String valToSend = "";
        valToSend += "AVG:";
        valToSend += String(angleMean);
        valToSend += ",SD:";
        valToSend += String(angleStdDev);
        valToSend += ",RNG:";
        valToSend += String(angleRange);
        int length = valToSend.length() + 1;
        char charBuf[length];
        valToSend.toCharArray(charBuf, length);
        ble.println(charBuf);
    }


    // Turn off "found field" indicator after <n> milliseconds
    delay(2000);
    digitalWrite(redBaseLED, LOW);

    return 0; // Normal

}

/* ----------------------------------------------
    runCal          : Run sensor calibration by removing
                        getting the filtered value of the
                        sensors without a magnetic field.
-----------------------------------------------*/
// int val          : 0 = Runs calibration
//                  : 1 = Prints cal values to console
//                  : 2 = Does 0 and then 1
// ----------------------------------------------
void runCal(int val){

    switch(val){
        case 1: // Output to console
            for(int i = 0; i < numAnalogInputs; i++) {
                Serial.print(sensorCalVal[i]);
                Serial.print(", ");
            }
            break;
        case 2: // Run cal then output to console
            filterVoltsLP(100);
            for(int i = 0; i < numAnalogInputs; i++) {
                sensorCalVal[i] = adcVolt[i][maxMinREG] - 1.0;
            }
            for(int i = 0; i < numAnalogInputs; i++) {
                Serial.print(sensorCalVal[i]);
                Serial.print(", ");
            }
            break;
        default: // Run normal cal
            filterVoltsLP(100);
            for(int i = 0; i < numAnalogInputs; i++) {
                sensorCalVal[i] = adcVolt[i][maxMinREG] - 1.0;
            }

    // Grab init values for filter
    fltrInitVal[0] = adcVolt[0][0];
    fltrInitVal[1] = adcVolt[0][1];
    fltrInitVal[2] = adcVolt[0][2];

    }
}

/* ----------------------------------------------
    setGetThresh     : Set or Get threshold value
                        maxima, minima, no-peak
-----------------------------------------------*/
// int readWrite_n  : 0 = write, 1 = read
// int setting      : Only valid on write, sets cal value from int
//                      to float value, Range: 0 to int max val,
//                      nominally 1 - 10.
// NOTEs            : Prints threshold value to console and app
// ----------------------------------------------
void setGetThresh(int readWrite_n, int setting){

    String valToSend = "";

    // Write value
    if (readWrite_n == 0){
        thresh = float(setting) * thresholdScaling;
        Serial.println(thresh);
        valToSend += String(thresh);
        int length = valToSend.length() + 1;
        char charBuf[length];
        valToSend.toCharArray(charBuf, length);
        ble.println(charBuf);

    }
    // Read value
    else{
        valToSend += String(thresh);
        int length = valToSend.length() + 1;
        char charBuf[length];
        valToSend.toCharArray(charBuf, length);
        ble.println(charBuf);
    }

}

/* ----------------------------------------------
    getBattVolts : Get LiPo battery voltage and send to app
-----------------------------------------------*/
// Gloabal          : battVin, vref, adcMaxTicks
// ----------------------------------------------
void getBattVolts() {

    float battVolts = 0.0;
    // x2 comes from the voltage divider
    battVolts = 2 * vref * (float(analogRead(battVin))/adcMaxTicks);
    String valToSend = "";
    valToSend += String(battVolts);
    int length = valToSend.length() + 1;
    char charBuf[length];
    valToSend.toCharArray(charBuf, length);
    ble.println(charBuf);

}

/*
    Project : HALO-1
    Author  : Jeff King
    Date    : 10/1/2018
*/

/* ----------------------------------------------
    convertToVolts  : Convert stored ADC raw tick values to voltages
-----------------------------------------------*/
// Gloabal          : numAnalogInputs, adcVolt, vref, adcTicks, adcMaxTicks, sensorCalVal
// ----------------------------------------------
void convertToVolts(int stackData) {

    if (stackData){
        for(int i = 0; i < numAnalogInputs; i++) {
            // Convert from ADC ticks to voltages and apply calibration settings
            adcVolt[i][3] += vref * (float(adcTicks[i])/adcMaxTicks) - sensorCalVal[i];
        }
    }
    else{
        for(int i = 0; i < numAnalogInputs; i++) {
            // Convert from ADC ticks to voltages and apply calibration settings
            adcVolt[i][3] = vref * (float(adcTicks[i])/adcMaxTicks) - sensorCalVal[i];
        }
    }

}

/* ----------------------------------------------
    findPeak    : Look for a min/max peak in the sampled data. This
                    function searches for a maxima or minima value depending on
                    the polarity of the detected magnetic field and
                    reports max/min/no peak found.
-----------------------------------------------*/
// Gloabal          : numAnalogInputs, analogInputPin
// Returns int      : 24 = no peak
//                  : +m = maxima
//                  : -m - 1 = minima (-1 accounts for sensor 0)
// NOTEs            : This function is strongly tied to getPeakType()
// ----------------------------------------------
int findPeak() {

    int foundMaxVal = 0; // Indicates maxima found
    int foundMinVal = 0; // Indicates minima found
    int m = 0;
    int n = 1;
    int peakState = 0;

    // Find maxima in the adcVolt data set
    while(foundMaxVal == 0) {
        if (adcVolt[m][maxMinREG] >= adcVolt[n][maxMinREG]) {
            n = n + 1;
            if (n >= numAnalogInputs) {
                foundMaxVal = 1;
            }
        }
        else {
            m = n;
            n = n + 1;
            if (n >= numAnalogInputs) {
                foundMaxVal = 1;
            }
        }
    }

    // If a sensor output is ABOVE the minimum threshold
    //  then move on
    if (adcVolt[m][maxMinREG] >= posThreshold){
        peakState = m;
    }
    else{

        m = 0;
        n = 1;

        while(foundMinVal == 0) {
            if (adcVolt[m][maxMinREG] <= adcVolt[n][maxMinREG]) {
                n = n + 1;
                if (n >= numAnalogInputs) {
                    foundMinVal = 1;
                }
            }
            else {
                m = n;
                n = n + 1;
                if (n >= numAnalogInputs) {
                    foundMinVal = 1;
                }
            }
        }

        // If a sensor output is BELOW the minimum threshold
        //  then move on
        if (adcVolt[m][maxMinREG] <= negThreshold){
            peakState = -(m) - 1;
        }
        // This indicates there is nothing detected by the sensor
        //  within the limits of the threshold
        else{
            peakState = 24;
        }

    }

    return peakState;

}

/* ----------------------------------------------
    getPeakType     : Decode the int value to determine type of peak;
                        maxima, minima, no-peak
-----------------------------------------------*/
// Returns int      : 0 = no peak
//                  : 1 = maxima
//                  : 2 = minima
// NOTEs            : This function is strongly tied to findPeak()
// ----------------------------------------------
int getPeakType(int m){

    int peakType; // Indicates type of peak: none, max, min
    if (m < 0){ // Minima peak found
        m = -(m + 1);
        peakType = 2;
    }
    else{
        if (m == 24){ // No peak found
            peakType = 0;
        }
        else{
        // else m >= 0 && m < 24, Maxima peak found
            peakType = 1;
        }
    }
    return peakType;

}
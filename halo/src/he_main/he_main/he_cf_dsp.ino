/*
    Project : HALO-1
    Author  : Jeff King
    Date    : 10/1/2018
*/

/* ----------------------------------------------
    filterVoltsLP : Apply low pass filter to collected data
-----------------------------------------------*/
// int numLoops     : Number of times to collect and filter data
// Gloabal          : numAnalogInputs, analogInputPin
// ----------------------------------------------
void filterVoltsLP(int numLoops) {

    // Filter the input
    // Digital filter : http://www.schwietering.com/jayduino/filtuino/
    //Low pass chebyshev filter order=1 alpha1=0.0008
    // Fs = 125kHz
    // Low Corner = 100 Hz
    float coeffA0 = 7.039041198087248041e-2;
    float coeffA1 = 0.85921917603825503917;

    for(int j = 0; j < numLoops; j++){
        // read the analog in values
        getAdcData(0);
        // Convert the raw values to voltages and store them in adcVolt[all][3]
        convertToVolts(0);
        for(int i = 0; i < numAnalogInputs; i++) {
            adcVolt[i][1] = adcVolt[i][2];
            adcVolt[i][2] = (coeffA0 * adcVolt[i][3]) + (coeffA1 * adcVolt[i][1]);
            adcVolt[i][0] = adcVolt[i][1] + adcVolt[i][2];
        }
    }
}

/* ----------------------------------------------
    setLPFilterInitCond : Set initial conditions for LP digital filter prior to
                            to filter use. This a calibration step must be done
                            prior to this function cal.
-----------------------------------------------*/
// Gloabal          : adcVolt, fltrInitVal, numAnalogInputs
// ----------------------------------------------
void setLPFilterInitCond(){
    for(int j = 0; j < 3; j++){
        for(int i = 0; i < numAnalogInputs; i++) {
            adcVolt[i][j] = fltrInitVal[j];
        }
    }
}

/* ----------------------------------------------
    CF_dataInit : Initialize the 2x3 point curve fitting array
-----------------------------------------------*/
// int m        : Points to the max/min value in the data set
// Returns int  : 0 = no boundary was crossed
//              : 1 = 0 degree boundary was crossed (value < 0 degrees)
//              : 2 = 360 degree boundary was crossed (value > 360 degrees)
// Gloabal      : CFMatrix, sensorFixedDegreesOffset, aosAngleOffs,
//                  adcVolt, maxMinREG
// ----------------------------------------------

int CF_dataInit(int m){
    // Number of samples to curve fit (always fixed, do not modify)
    // int numCFSamples = 3;

    // Create 2xN matrices (N = numCFSamples)
    // float CFMatrix [2][numCFSamples];

    int boundaryXed = 0;
    // Assign 2x3 matrices (row 0 = angle, row 1 = magnitude)
    // This code grabs the 3 largest values with the peak in the middle.
    // It also corrects for the mathematical problem of being able to properly
    //  account for the angle when crossing a 0 or 360 degree boundary.
    // NOTE: The commented code is for 5 samples where the current is 3 samples
    switch (m) {
        case 0:
            // for(int i = 0; i < numCFSamples - 3; i++) {
                    // CFMatrix[0][i] = float((-2 + i) * sensorFixedDegreesOffset) + aosAngleOffs;
                    // CFMatrix[1][i] = adcVolt[22 + i][maxMinREG];
                // }
            CFMatrix[0][0] = float((-1) * sensorFixedDegreesOffset) + aosAngleOffs;
            CFMatrix[1][0] = adcVolt[23][maxMinREG];
            // for(int i = 0; i < numCFSamples - 2; i++) {
                // CFMatrix[0][i + 2] = float((m + i) * sensorFixedDegreesOffset) + aosAngleOffs;
                // CFMatrix[1][i + 2] = adcVolt[m + i][maxMinREG];
            // }
            for(int i = 0; i < numCFSamples - 1; i++) {
                CFMatrix[0][i + 1] = float((m + i) * sensorFixedDegreesOffset) + aosAngleOffs;
                CFMatrix[1][i + 1] = adcVolt[m + i][maxMinREG];
            }
            boundaryXed = 1;
            break;

        // case 1:
            // CFMatrix[0][0] = float((-1) * sensorFixedDegreesOffset) + aosAngleOffs;
            // CFMatrix[1][0] = adcVolt[23][maxMinREG];
            // for(int i = 0; i < numCFSamples - 1; i++) {
                // CFMatrix[0][i + 1] = float((m - 1 + i) * sensorFixedDegreesOffset) + aosAngleOffs;
                // CFMatrix[1][i + 1] = adcVolt[m - 1 + i][maxMinREG];
            // }
            // break;

        // case 22:
            // for(int i = 0; i < numCFSamples - 1; i++) {
                // CFMatrix[0][i] = float((m - 2 + i) * sensorFixedDegreesOffset) + aosAngleOffs;
                // CFMatrix[1][i] = adcVolt[m - 2 + i][maxMinREG];
            // }
            // CFMatrix[0][4] = float((24) * sensorFixedDegreesOffset) + aosAngleOffs;
            // CFMatrix[1][4] = adcVolt[0][maxMinREG];
            // break;

        case 23:
            // for(int i = 0; i < numCFSamples - 2; i++) {
                // CFMatrix[0][i] = float((m - 2 + i) * sensorFixedDegreesOffset) + aosAngleOffs;
                // CFMatrix[1][i] = adcVolt[m - 2 + i][maxMinREG];
            // }
            for(int i = 0; i < numCFSamples - 1; i++) {
                CFMatrix[0][i] = float((m - 1 + i) * sensorFixedDegreesOffset) + aosAngleOffs;
                CFMatrix[1][i] = adcVolt[m - 1 + i][maxMinREG];
            }
            // for(int i = 0; i < numCFSamples - 3; i++) {
                // CFMatrix[0][i + 3] = float((24 + i) * sensorFixedDegreesOffset) + aosAngleOffs;
                // CFMatrix[1][i + 3] = adcVolt[i][maxMinREG];
            // }
            CFMatrix[0][2] = float((24) * sensorFixedDegreesOffset) + aosAngleOffs;
            CFMatrix[1][2] = adcVolt[0][maxMinREG];
            boundaryXed = 2;
            break;

        default:
            for(int i = 0; i < numCFSamples; i++) {
                // CFMatrix[0][i] = float((m - 2 + i) * sensorFixedDegreesOffset) + aosAngleOffs;
                // CFMatrix[1][i] = adcVolt[m - 2 + i][maxMinREG];
                CFMatrix[0][i] = float((m - 1 + i) * sensorFixedDegreesOffset) + aosAngleOffs;
                CFMatrix[1][i] = adcVolt[m - 1 + i][maxMinREG];
            }
            break;
        }
        
    // Return boundary crossing indicator
    return boundaryXed;

}

/* ----------------------------------------------
    normCF_Data : Normalize CFMatrix Data
-----------------------------------------------*/
// Gloabal  : CFMatrix
// ----------------------------------------------
void normCF_Data(){

    float normVal = CFMatrix[1][1];
    for(int i = 0; i < numCFSamples; i++) {
        CFMatrix[1][i] = CFMatrix[1][i]/normVal;
    }
}


/* ----------------------------------------------
    getAngle : Initialize the 2x3 point curve fitting array
-----------------------------------------------*/
// int boundaryXed  : 0 = no boundary was crossed
//                  : 1 = 0 degree boundary was crossed (value < 0 degrees)
//                  : 2 = 360 degree boundary was crossed (value > 360 degrees)
// int peakType     : 0 = no peak (undefined)
//                  : 1 = maxima peak was found
//                  : 2 = minima peak was found
// Returns float    : Angle of detected magnetic field
// Gloabal          : CFMatrix, sensorFixedDegreesOffset, aosAngleOffs,
//                      adcVolt, maxMinREG
// ----------------------------------------------
float getAngle(int boundaryXed, int peakType){

    // If a boundary has been crossed add/subtract a 180 degree offset to keep the
    //  angle within the limits (0-360) of the curve fitting algorithm.
    // This offset will be removed after the algorithm has been ran
    if (boundaryXed > 0) {
        for(int i = 0; i < numCFSamples; i++) {
            if (boundaryXed == 1) {
                CFMatrix[0][i] += 180.0;
            }
            else{
                if (boundaryXed == 2) {
                    CFMatrix[0][i] -= 180.0;
                }
            }
        }
    }

    /* **********************************
        Curve fit (CF)
       ******************************** */
    // This code uses linear algebra to curve fit the data points to
    //  calculate a closer approximation of where the magnetic field is detected.
    // NOTES: Copied the matlab notes to help show the linear algebra flow
    //
    // A X = B
    //
    // -- -- -- -- -- --
    // | n Σxi Σxi^2 | | a0 | | Σyi |
    // | Σxi Σxi^2 Σxi^3 | | a1 | = | Σ(xi yi) |
    // | Σxi^2 Σxi^3 Σxi^4 | | a2 | | Σ(xi^2 yi) |
    // -- -- -- -- -- --
    //
    // ... variables ...
    //
    // -- -- -- -- -- --
    // | A11 A12 A13 | | x0 | | B11 |
    // | A21 A22 A23 | | x1 | = | B21 |
    // | A31 A32 A33 | | x2 | | B31 |
    // -- -- -- -- -- --
    // Calculate terms of above matricies

    // test vs matlab outputs, final angle should be 80.1 degrees
    // CFMatrix [0][0] = 50.0;
    // CFMatrix [0][1] = 65.0;
    // CFMatrix [0][2] = 80.0;
    // CFMatrix [0][3] = 95.0;
    // CFMatrix [0][4] = 110.0;
    // CFMatrix [1][0] = 0.4444;
    // CFMatrix [1][1] = 0.9778;
    // CFMatrix [1][2] = 1.0;
    // CFMatrix [1][3] = 0.7778;
    // CFMatrix [1][4] = 0.5556;

    // Test for a sample of 3 instead of 5
    // CFMatrix [0][0] = 65.0;
    // CFMatrix [0][1] = 80.0;
    // CFMatrix [0][2] = 95.0;
    // CFMatrix [1][0] = 1.9778;
    // CFMatrix [1][1] = 2.0;
    // CFMatrix [1][2] = 1.7778;

    float numSamples = float(numCFSamples);
    float A11 = numSamples;
    float A12 = getSumFloat(CFMatrix[0], numCFSamples);
    float A13 = getSumPowFloat(CFMatrix[0], numCFSamples, 2);
    float A21 = A12;
    float A22 = A13;
    float A23 = getSumPowFloat(CFMatrix[0], numCFSamples, 3);
    float A31 = A22;
    float A32 = A23;
    float A33 = getSumPowFloat(CFMatrix[0], numCFSamples, 4);

    // Use ZXX instead of BXX since BXX is reserved
    float Z11 = getSumFloat(CFMatrix[1], numCFSamples);
    float Z21 = 0.0;
    float Z31 = 0.0;

    // float B21 = getSumFloat(CFMatrix[0] * CFMatrix[1]);
    for(int i = 0; i < numCFSamples; i++) {
        Z21 = Z21 + CFMatrix[0][i] * CFMatrix[1][i];
    }

    // float B31 = getSumPowFloat(dA(:,1).^2.*dA(:,2));
    for(int i = 0; i < numCFSamples; i++) {
        Z31 = Z31 + pow(CFMatrix[0][i], 2) * CFMatrix[1][i];
    }

    // Use the BLA library for linear algebra
    // https://github.com/tomstewart89/BasicLinearAlgebra

    // Create the A matrix
    BLA::Matrix<3,3> A = {A11, A12, A13,
                          A21, A22, A23,
                          A31, A32, A33};

    // Create the B matrix
    BLA::Matrix<3,1> B = {Z11,
                          Z21,
                          Z31};

    // Create the X matrix
    BLA::Matrix<3,1> X = A.Inverse() * B;

    // yx=X(1) + X(2).*xx + X(3).*xx.^2; %Y-axis data
    // Instead of a separate array for xx it was put directly into the following
    //  equation to minimize RAM usage
    float yx [3600] = {0};
    for(int i = 0; i < 3600; i++){
        yx[i] = (X(0)) + (X(1) * float(i)*0.1) + (X(2) * pow(float(i)*0.1, 2));
    }

    // Find the position of the max/min depending on the type of peak
    float maxCFVal = yx[0]; // Assign it the first value of the array
    int maxCFPoint = 0;
    int positionOfAngle = 0;
    if (peakType == 1){ // find position of maxima peak
        for(int i = 0; i < 3599; i++){
            if (maxCFVal <= yx[i + 1]){
                maxCFVal = yx[i + 1];
                positionOfAngle = i + 1;
            }
        }
    }
    else{ // find position of minima peak
        for(int i = 0; i < 3599; i++){
            if (maxCFVal >= yx[i + 1]){
                maxCFVal = yx[i + 1];
                positionOfAngle = i + 1;
            }
        }
    }

    // Remove angle offset if there is one
    if (boundaryXed == 1) {
        maxCFVal = float(positionOfAngle - 1800)*0.1;
    }
    else{
        if (boundaryXed == 2) {
            maxCFVal = float(positionOfAngle + 1800)*0.1;
        }
        else{
            maxCFVal = float(positionOfAngle)*0.1;
        }
    }

    // Return angle of magnetic field detection
    return maxCFVal;

}
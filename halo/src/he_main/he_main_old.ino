// System clock is 16 MHz

//------------------------------------------------------------------------
/*                  ADC Conversion time settings                        */
//------------------------------------------------------------------------
// ADC Definitions to set the conversion time
// The different settings set the ADC clock frequency, the conversion time
// and the equivalent number of bits
// ADC_TIME_104    f=125kHz   Tconv=104us    ENOB > 9
// ADC_TIME_52     f=250kHz   Tconv=52us     ENOB > 9
// ADC_TIME_26     f=500kHz   Tconv=26us     ENOB > 9
// ADC_TIME_13     f=1MHz     Tconv=13us     ENOB > 8
#define ADC_TIME_104  ADCSRA=(ADCSRA&0xF80)|0x07
#define ADC_TIME_52   ADCSRA=(ADCSRA&0xF80)|0x06
#define ADC_TIME_26   ADCSRA=(ADCSRA&0xF80)|0x05
#define ADC_TIME_13   ADCSRA=(ADCSRA&0xF80)|0x04
//------------------------------------------------------------------------

// ADC metrics
const float adcMaxTicks = 1023;
const float vref = 3.3;

// ADC raw tick values
int adcTicks[3] = {0, 0, 0};

// ADC converted raw voltage values
float adcVolt[3][4] = {
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

// Number of analog intputs
const int numAnalogInptus = 3;

// ADC Pin assignments
const int analogInputPin[numAnalogInptus] = {A0, A1, A2}; // Analog input pin that the potentiometer is attached to

// Calibration sensor output voltages
const float sensorCalVal[numAnalogInptus] = {1.00, 1.00, 1.02};

// Number of sensors
const int numSensors = 24;
// Sensor separation in degrees, if remainder is not zero this will need to change types
const int sensorFixedDegreesOffset = 360/numSensors;

void setup() {

    // Setup sampling time
    ADC_TIME_104;  // Normal Aduino Setting (104us)

    // Set analog reference voltage based on vref
    if (vref == 3.3) {
        analogReference(EXTERNAL);
    }
    else {
        analogReference(DEFAULT);
    }

    // initialize serial communications at 9600 bps:
    Serial.begin(115200);

}

void loop() {

    // read the analog in values
    for(int i = 0; i < numAnalogInptus; i++) {
        adcTicks[i] = analogRead(analogInputPin[i]);
    }

    // Convert the raw values to voltages
    for(int i = 0; i < numAnalogInptus; i++) {
        adcVolt[i][3] = vref * (float(adcTicks[i])/adcMaxTicks);
    }


    // Filter the input
    // Digital filter : http://www.schwietering.com/jayduino/filtuino/
    //Low pass chebyshev filter order=1 alpha1=0.0008
    // Fs = 125kHz
    // Low Corner = 100 Hz
    float coeffA0 = 7.039041198087248041e-2;
    float coeffA1 = 0.85921917603825503917;

    for(int i = 0; i < numAnalogInptus; i++) {
        adcVolt[i][1] = adcVolt[i][2];
        adcVolt[i][2] = (coeffA0 * adcVolt[i][3]) + (coeffA1 * adcVolt[i][1]);
        adcVolt[i][0] = adcVolt[i][1] + adcVolt[i][2];
    }

    //***************************************************************************
    // Find maximum value, use adcVolt
    // If two maxima is found then use the first one (lowest array #)
    //***************************************************************************
    int foundMaxVal = 0; // Indicates maxima found
    int found2Vals = 0; // Indicates two values found that were equal
    int m = 0;
    int n = 1;
    while(foundMaxVal == 0) {
        if (adcVolt[m][0] >= adcVolt[n][0]) {
            n = n + 1;
            if (n >= numAnalogInptus) {
                foundMaxVal = 1;
                if (adcVolt[m][0] == adcVolt[n][0]) {
                    found2Vals = 1;
                    n = n - 1;
                }
            }
        }
        else {
            m = m + 1;
            if (m >= numAnalogInptus) {
                foundMaxVal = 1;
                m = m - 1;
            }
            n = n + 1;
        }
    }

    //***************************************************************************
    // Find degrees of difference
    //***************************************************************************

    int sensorNumMax0 = m; // This is the first sensor that has a maxima
    int sensorNumMax1 = n; // This is the second sensor that has a maxima, only valid if two maxima are found
    int offsetDeg = 0; // Degrees of calculated offset of H-field

    // Set base offset in degrees
    offsetDeg = sensorNumMax0 * sensorFixedDegreesOffset;

    // If equal values then H-field is equally spaced between two sensors
    if (found2Vals == 1) {
        // Check if the other equal sensor is one array position + 1
        if (sensorNumMax1 == sensorNumMax0 + 1) {
            offsetDeg = offsetDeg + sensorFixedDegreesOffset/2;
        }
        // Must be array positions 1 and numAnalogInptus
        else {
            offsetDeg = 360 - sensorFixedDegreesOffset/2;
        }
    }
    // H-field is not equal spaced between two sensors
    else {

        // Check if maxima sensor is on low or high array boundary
        switch (sensorNumMax0) {
            case 0: // Lower array boundary
                if (adcVolt[numAnalogInptus - 1][0] > adcVolt[1][0]) {
                    offsetDeg = (sensorFixedDegreesOffset * numSensors) - int(float(sensorFixedDegreesOffset) * (adcVolt[numAnalogInptus][0] - sensorCalVal[numAnalogInptus]) / (adcVolt[sensorNumMax0][0] - sensorCalVal[numAnalogInptus]));
                }
                else {
                    offsetDeg = int(float(sensorFixedDegreesOffset) * (adcVolt[1][0] - sensorCalVal[1]) / (adcVolt[sensorNumMax0][0] - sensorCalVal[1]));
                }
                break;
            case (numAnalogInptus - 1): // Upper array boundary
                if (adcVolt[sensorNumMax0 - 1][0] > adcVolt[0][0]) {
                    offsetDeg = (sensorNumMax0 * sensorFixedDegreesOffset) - int(float(sensorFixedDegreesOffset) * (adcVolt[sensorNumMax0 - 1][0] - sensorCalVal[sensorNumMax0 - 1]) / (adcVolt[sensorNumMax0][0] - sensorCalVal[sensorNumMax0 - 1]));
                }
                else {
                    offsetDeg = (sensorNumMax0 * sensorFixedDegreesOffset) + int(float(sensorFixedDegreesOffset) * (adcVolt[numAnalogInptus][0] - sensorCalVal[numAnalogInptus]) / (adcVolt[sensorNumMax0][0] - sensorCalVal[numAnalogInptus]));
                }
                break;
            default: // Not on an array boundary
                // Check if H-field is dead centered
                if (adcVolt[sensorNumMax0 - 1][0] == adcVolt[sensorNumMax0 + 1][0]) {
                    // Do nothing, angle is already set correctly
                }
                else {
                    if (adcVolt[sensorNumMax0 - 1][0] > adcVolt[sensorNumMax0 + 1][0]) {
                        offsetDeg = offsetDeg - int(float(sensorFixedDegreesOffset) * (adcVolt[sensorNumMax0 - 1][0] - sensorCalVal[sensorNumMax0 - 1]) / (adcVolt[sensorNumMax0][0] - sensorCalVal[sensorNumMax0 - 1]));
                        }
                    else {
                        offsetDeg = offsetDeg +  int(float(sensorFixedDegreesOffset) * (adcVolt[sensorNumMax0 + 1][0] - sensorCalVal[sensorNumMax0 + 1]) / (adcVolt[sensorNumMax0][0] - sensorCalVal[sensorNumMax0 + 1]));
                    }
                }
        }
    }



    // print the results to the Serial Monitor:
    Serial.print("sensor = ");
    Serial.print(adcVolt[0][0]);
    Serial.print("V\t ");
    Serial.print(adcVolt[1][0]);
    Serial.print("V\t ");
    Serial.print(adcVolt[2][0]);
    Serial.print("V\t ");
    Serial.print("Max = A");
    Serial.print(m);
    Serial.print("  ");
    Serial.print("Angle = ");
    Serial.print(offsetDeg);
    Serial.print("'");
    Serial.println("");

    // wait 2 milliseconds before the next loop for the analog-to-digital
    // converter to settle after the last reading:
    delay(200);
}



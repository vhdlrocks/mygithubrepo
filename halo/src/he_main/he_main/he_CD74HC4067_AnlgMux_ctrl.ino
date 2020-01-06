/*
    Project : HALO-1
    Author  : Jeff King
    Date    : 10/1/2018
*/

/* ----------------------------------------------
    adcSelect : Selects the desired ADC channel
-----------------------------------------------*/
// int adcNum   : ADC Channel select
// ----------------------------------------------
void adcSelect(int adcNum){
    switch (adcNum) {
        case 0: // First mux
            digitalWrite(s3, LOW); digitalWrite(s2, LOW); digitalWrite(s1, LOW); digitalWrite(s0, LOW);
            break;
        case 1:
            digitalWrite(s3, LOW); digitalWrite(s2, LOW); digitalWrite(s1, LOW); digitalWrite(s0, HIGH);
            break;
        case 2:
            digitalWrite(s3, LOW); digitalWrite(s2, LOW); digitalWrite(s1, HIGH); digitalWrite(s0, LOW);
            break;
        case 3:
            digitalWrite(s3, LOW); digitalWrite(s2, LOW); digitalWrite(s1, HIGH); digitalWrite(s0, HIGH);
            break;
        case 4:
            digitalWrite(s3, LOW); digitalWrite(s2, HIGH); digitalWrite(s1, LOW); digitalWrite(s0, LOW);
            break;
        case 5:
            digitalWrite(s3, LOW); digitalWrite(s2, HIGH); digitalWrite(s1, LOW); digitalWrite(s0, HIGH);
            break;
        case 6:
            digitalWrite(s3, LOW); digitalWrite(s2, HIGH); digitalWrite(s1, HIGH); digitalWrite(s0, LOW);
            break;
        case 7:
            digitalWrite(s3, LOW); digitalWrite(s2, HIGH); digitalWrite(s1, HIGH); digitalWrite(s0, HIGH);
            break;
        case 8:
            digitalWrite(s3, HIGH); digitalWrite(s2, LOW); digitalWrite(s1, LOW); digitalWrite(s0, LOW);
            break;
        case 9:
            digitalWrite(s3, HIGH); digitalWrite(s2, LOW); digitalWrite(s1, LOW); digitalWrite(s0, HIGH);
            break;
        case 10:
            digitalWrite(s3, HIGH); digitalWrite(s2, LOW); digitalWrite(s1, HIGH); digitalWrite(s0, LOW);
            break;
        case 11:
            digitalWrite(s3, HIGH); digitalWrite(s2, LOW); digitalWrite(s1, HIGH); digitalWrite(s0, HIGH);
            break;
        case 12:
            digitalWrite(s3, HIGH); digitalWrite(s2, HIGH); digitalWrite(s1, LOW); digitalWrite(s0, LOW);
            break;
        case 13:
            digitalWrite(s3, HIGH); digitalWrite(s2, HIGH); digitalWrite(s1, LOW); digitalWrite(s0, HIGH);
            break;
        case 14:
            digitalWrite(s3, HIGH); digitalWrite(s2, HIGH); digitalWrite(s1, HIGH); digitalWrite(s0, LOW);
            break;
        case 15:
            digitalWrite(s3, HIGH); digitalWrite(s2, HIGH); digitalWrite(s1, HIGH); digitalWrite(s0, HIGH);
            break;
        case 16: // Second mux
            digitalWrite(s3, LOW); digitalWrite(s2, LOW); digitalWrite(s1, LOW); digitalWrite(s0, LOW);
            break;
        case 17:
            digitalWrite(s3, LOW); digitalWrite(s2, LOW); digitalWrite(s1, LOW); digitalWrite(s0, HIGH);
            break;
        case 18:
            digitalWrite(s3, LOW); digitalWrite(s2, LOW); digitalWrite(s1, HIGH); digitalWrite(s0, LOW);
            break;
        case 19:
            digitalWrite(s3, LOW); digitalWrite(s2, LOW); digitalWrite(s1, HIGH); digitalWrite(s0, HIGH);
            break;
        case 20:
            digitalWrite(s3, LOW); digitalWrite(s2, HIGH); digitalWrite(s1, LOW); digitalWrite(s0, LOW);
            break;
        case 21:
            digitalWrite(s3, LOW); digitalWrite(s2, HIGH); digitalWrite(s1, LOW); digitalWrite(s0, HIGH);
            break;
        case 22:
            digitalWrite(s3, LOW); digitalWrite(s2, HIGH); digitalWrite(s1, HIGH); digitalWrite(s0, LOW);
            break;
        case 23:
            digitalWrite(s3, LOW); digitalWrite(s2, HIGH); digitalWrite(s1, HIGH); digitalWrite(s0, HIGH);
            break;
        default:
            digitalWrite(s3, LOW); digitalWrite(s2, LOW); digitalWrite(s1, LOW); digitalWrite(s0, LOW);
        }
        // This delay value should be enough time for the mux to settle based on the datasheet
        delayMicroseconds(1);
  return;
}

/* ----------------------------------------------
    getAdcData : Get ADC data from the analog mux
-----------------------------------------------*/
// int stackData    : Add new data to the old data
// Gloabal          : adcTicks, numAnalogInputs, analogInputPin
// ----------------------------------------------
void getAdcData(int stackData) {

    // read the analog in values
    int analogPinLocal;
    for(int i = 0; i < numAnalogInputs; i++) {
        // Call function to select ADC input
        adcSelect(i);
        if (i < 16) {
            analogPinLocal = 0;
        }
        else {
            analogPinLocal = 1;
        }
        // Call function to get ADC data
        if (stackData) {// sum the old data with the new
            adcTicks[i] += analogRead(analogInputPin[analogPinLocal]);
        }
        else {
            adcTicks[i] = analogRead(analogInputPin[analogPinLocal]);
        }

    }
}


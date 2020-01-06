/*
    Project : HALO-1
    Author  : Jeff King
    Date    : 10/1/2018

    Todo by Jeff:
   *1. Make the sensors bi-polar (can only curve fit positve voltages)
   *2. D9 is currently being used as a control bit for the ADC mux. D9 is also
        an analog input that reads the LiPo battery voltage. Change this pin to another DIO.
        D12 would be a better DIO to use.
   *3. Add BLE functionality.
        a. Define what commands will be RX on this side.
   *4. Wire up the two BASE LEDs (bottom of case) to DIO pin A4 and analog input A5.
        a. Need one to indicate a BLE connection
        b. Other one can indicate a sensor is being detected or general purpose.
   *5. Update filter co-efficients to be faster.
   *6. Add a self calibration function initiated by the App or on start-up.
   *7. Add battery voltage read-out (after #2 is complete).
   *8. Measure time it takes to process one search.
   -9. Plot angle in matlab and find the standard deviation.
        >>Matt can do this in the field
   *10. Add adjustable threshold from app.
   *11. Refactor to make code more readable.
   -12. Add calibration values to Flash memory so calibration doesn't have to be done every time
        the unit restarts.
        >> Will leave in RAM, cal should be done each time the unit starts up.
    13. Change BLE name to Halo-1<serial number>
    14. Return the command with each BLE reply as per Josh's request.
   *15. Look into a better way of getting AOS sensor position to apply offset. This might be the reason
        Matt was seeing a 5 degree variation during testing with his aluminum test fixture.
        >> Capturing AOS data right after a sensor has been found, instead of before.
   -16. Allow the user to set the 0 degree position to any value. This could also be done in the app if needed.
        >> Should always be aligned to gravity
   -17. Use only positive values between 0 and 359.999 degrees to represent angles.
        >> Set between -270.0 and 360.0 instead
   *18. Set final angle of zero to read with respect to gravity and a magnet on the top of the circle.
        180 degrees would be a magnet on the bottom of the circle wrt gravity.
        >> The limitation here is that the DUT can never be parallel to the angle of gravity, only horizontal (perpendicular to gravity).

    Todo by Matt:
   *1. Get heatshrink on switch shell connected wires for protection and stability.
    2. Provide method to keep sensors 0 and 23 at the same height as the others.
    3. Update chassis to allow clamp to fully close all of the time.
*/

// Required libraries for the BNO055 AOS Sensor
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

// Required libraries for the Adafruit_BluefruitLE_nRF51 BLE module
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

// Use BasicLinearAlgebra Library for matices manipulation
// Used in he_df_dsp
#include <BasicLinearAlgebra.h>

// All the functions in BasicLinearAlgebra are wrapped up inside the namespace BLA
using namespace BLA;

// System clock is 48 MHz

/* ************************************
          HALO-1 System Debug Setting(s)
************************************ */
// 0 - Not in debug mode
// 1 - Debug mode, won't run until a serial connection is present
const int debugMode = 0;

/* ************************************
          HALO-1 Pinout START
************************************ */

//SparkFun Analog/Digital MUX Breakout - CD74HC4067
const int s0 = 5;   // mux select bit 0
const int s1 = 6;   // mux select bit 1
const int s2 = 12;  // mux select bit 2
const int s3 = 10;  // mux select bit 3

const int aos_rst = 11; // AOS (absolute orientation sensor) reset active high

const int led = 13;     // Red LED on Feather M0 board, pin 13, active high

const int redBaseLED = A5;  // Handle base LED
const int blueBaseLED = A4; // Handle base LED

const int battVin = 9;  // LiPo Battery Voltage

/* ************************************
          ADC Setup
************************************ */

// ADC metrics
const int numAdcBits = 10; // Number of ADC bits
const float adcMaxTicks = (pow(2, numAdcBits) - 1);
const float vref = 3.3;

// Used for setting the threshold via app (used by func setGetThresh)
const float thresholdScaling = 0.025;
// Magnetic detection thresholds
float thresh = 5.0 * thresholdScaling;
float posThreshold = 1.000 + thresh;
float negThreshold = 1.000 - thresh;

// Number of analog intputs
const int numAnalogInputs = 24;

// ADC raw tick values
int adcTicks[numAnalogInputs] = {0};

// Setting: which ADC value to use in DSP, unfiltered = 3, filtered = 0
const int maxMinREG = 3;

// Setting: which ADC value to use in "print", unfiltered = 3, filtered = 0
const int printADCVal = maxMinREG;

// ADC converted raw voltage values
float adcVolt[numAnalogInputs][4] = {0};

// Low Pass Filter initial conditions after calibration
float fltrInitVal[4] = {0};

// ADC Pin assignments
const int analogInputPin[2] = {A0, A1}; // Analog input pin that the potentiometer is attached to

// Calibration sensor output voltages (make this self calibrating)
float sensorCalVal[numAnalogInputs] = {0.05, 0.02, 0.01, 0.00, 0.04, 0.00,
                                             0.01, 0.00, 0.00, 0.02, 0.00, 0.02,
                                             0.04, 0.02, 0.01, 0.02, 0.03, 0.01,
                                             0.01, 0.02, 0.07, 0.01, 0.02, 0.01,};
// const float sensorCalVal[numAnalogInputs] = {1.00};

// Number of sensors
const int numSensors = 24;
// Sensor separation in degrees, if remainder is not zero this will need to change types
const int sensorFixedDegreesOffset = 360/numSensors;

int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

/* ************************************
    AOS Adafruit BNO055 Sensor
************************************ */
// AOS call
Adafruit_BNO055 bno = Adafruit_BNO055(55);
sensors_event_t event; // Declare an AOS event object

/* ************************************
    Curve Fitting (CF) Metrics
************************************ */
// Number of samples to curve fit (always fixed, do not modify)
const int numCFSamples = 3;

// Create 2xN matrices (N = numCFSamples)
float CFMatrix [2][numCFSamples];

// AOS Y-axis angle
float aosAngleOffs = 0.0;

/* ************************************
    Adafruit BluefruitLE nRF51 settings
************************************ */
/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ************************************
          System Setup
************************************ */

void setup() {

    /* **********************************
        Initialize I/O
       ******************************** */
    pinMode(redBaseLED, OUTPUT);
    pinMode(blueBaseLED, OUTPUT);
    // Setup board LED indicator
    pinMode(led, OUTPUT);

    // Setup and Deassert AOS reset
    pinMode(aos_rst, OUTPUT);
    digitalWrite(aos_rst, HIGH);

    // Setup and put analog mux in default state
    pinMode(s0, OUTPUT);
    pinMode(s1, OUTPUT);
    pinMode(s2, OUTPUT);
    pinMode(s3, OUTPUT);
    digitalWrite(s0, LOW);
    digitalWrite(s1, LOW);
    digitalWrite(s2, LOW);
    digitalWrite(s3, LOW);

    /* **********************************
    Set analog reference voltage based on vref
       ******************************** */
    if (vref != float(3.3)) {
        analogReference(AR_EXTERNAL);
    }

    if (debugMode){
        while (!Serial) ; // wait for serial debug port to connect to PC
    }
    // initialize Serial communications
    Serial.begin(115200);

    /* **********************************
        Initialize AOS Sensor
       ******************************** */
    init_aos();

    /* **********************************
        Initialize Adafruit BluefruitLE nRF51
       ******************************** */
    init_nRF51();

}

/* ************************************
          MAIN
************************************ */

void loop() {

    // Make sure BLE is connected, if not, re-init
    if (!ble.isConnected()){
        reconnect_nRF51();
    }

    // Set to 1 to execute DSP function
    int excecuteDSP = 0;

    /* **********************************
        Debug:
        Read from console and send to nRF51
       ******************************** */
    // Check for user input
    char n, inputs[BUFSIZE+1];

    if (Serial.available())
    {
        n = Serial.readBytes(inputs, BUFSIZE);
        inputs[n] = 0;
        // Send characters to Bluefruit
        Serial.print("Sending: ");
        Serial.println(inputs);

        // Send input data to host via Bluefruit
        ble.print(inputs);
    }


    /* **********************************
        Read nRF51 data
       ******************************** */

    int cmdWordCnt = 0;
    const int bleRxBufferSize = 20; //Fixed array size for all cmd words
    char rxWord[bleRxBufferSize];   //Copy of cmd word and parameters
    char rxCmd[bleRxBufferSize];    //Copy of cmd word only
    const int bleParamLimit = 3;    //Maximum number of BLE cmd word parameters
    char bleParam[bleParamLimit][bleRxBufferSize/2]; //Copy of cmd word
    int bleParamInt[bleParamLimit] = {0};       //Integer values of passed bleParam
    int bleParamCharCnt[bleParamLimit] = {0};   // Counts the number of chars in parameter
    int bleDelimiter = int(';');    //cmd delimeter
    int blePreamble = int('*');     //cmd preamble
    int bleSeperator = int(',');    //parameter separator
    int preambleDetected = 0;       // Set to 1 when preamble is detected
    int bleParamFound = 0;          // Set to 1 when cmd parameter found
    int bleParamCnt = 0;            // Counts the number of parameters sent

    //***********************************
    // List of valid HALO-1 BLE Commands and definitions
    /*

    ---------------------------------------------------------------------------
    --- Read battery voltage
    ---------------------------------------------------------------------------
    CMD: *batt,0;
    RTRN: >*batt,3.81;

    ---------------------------------------------------------------------------
    --- Initiate Calibration
    ---------------------------------------------------------------------------
    CMD: *cal,0;
    RTRN: >*cal;

    ---------------------------------------------------------------------------
    --- Write threshold
    ---------------------------------------------------------------------------
    Params: *thresh,PARAM0,PARAM1;
    PARAM0 = int value: 0 = Write
                        1 = Read
    PARAM1 (only for writes) = int value - range 1 - 10, 5 is normal
    Write example
    CMD: *thresh,0,5;
    RTRN: >*thresh,0.12;
        Note: Returns voltage offset 1.0 + return_val = 1.12V in example
    Read example
    CMD: *thresh,1;
    RTRN: >*thresh,0.12;
        Note: Returns voltage offset 1.0 + return_val = 1.12V in example

    ---------------------------------------------------------------------------
    --- Search for magnet
    ---------------------------------------------------------------------------
    Params: *search,PARAM0,PARAM1,PARAM2;
    PARAM0 = int value: Number of loops to search until found - 1-MAX_INT_RANGE
    PARAM1 = int value: 0 = Normal (don't return an angle with each loop)
                            Will return angles of each loop if PARAM2 = 0
                        1 = Returns an angle with each loop PARAM0 times
    PARAM2 = int value: 0 = Normal, return a single angle or last angle when
                            PARAM0 = 1
                        1 = Return mean value of all of the loops from PARAM0
                        2 = Return mean, standard deviation and range of all angles
    Search examples:

    Search until 1 value is found and return angle (Good for searching only)
    CMD: *search,1,0,0;
    RTRN: >*search,6.70;

    Search 3 times and report each angle (Good for debugging)
    CMD: *search,3,0,0;
    RTRN: >*search,6.70,6.60,6.70;

    Search 3 times and report mean angle, angle standard deviation and range of angles (Good for debugging or dwelling)
    CMD: *search,3,0,2;
    RTRN: >*search,AVG:6.74,SD:0.10,RNG:0.30;

    Search 7 times and report angle simple average (Good for debugging or dwelling)
    CMD: *search,7,0,1;
    RTRN: >*search,AVG:6.74;

    Search 4 times and report angle simple average (Good for debugging or dwelling)
    CMD: *search,4,0,1;
    RTRN: >*search,AVG:6.74;

    */

    // Test start
    char cmdStart[bleRxBufferSize] = "*start";
    // Search for magnetic field, light up BASE <RED/BLUE> LED when found
    char cmdSearch[bleRxBufferSize] = "*search";
    // Get angle of magnetic field
    char cmdAngle[bleRxBufferSize] = "*angle";
    // Calibrate sensors
    char cmdCal[bleRxBufferSize] = "*cal";
    // Get battery voltage
    char cmdBattVolt[bleRxBufferSize] = "*batt";
    // Get temp
    char cmdTemp[bleRxBufferSize] = "*temp";
    // Set threshold for peak sensor detection (input is 1 - 10)
    char cmdThresh[bleRxBufferSize] = "*thresh";

    int loopDSP = 0;
    // Echo received data
    while ( ble.available() )
    {

        int c = 0;
        while (c != bleDelimiter){
            if (ble.available()){
                c = ble.read();
                if (c == blePreamble || preambleDetected == 1){
                    preambleDetected = 1;
                    if (c != bleDelimiter){
                        rxWord[cmdWordCnt] = char(c);
                        cmdWordCnt++;
                        if (c == bleSeperator){
                            bleParamFound = 1;
                            if (bleParamCnt == 0){
                                for(int i = 0; i < bleRxBufferSize; i++){
                                    rxCmd[i] = rxWord[i];
                                }
                                rxCmd[cmdWordCnt - 1] = '\0';
                            }
                            bleParamCnt++;
                        }
                        else{
                            if (bleParamCnt > 0){
                                // Load parameters
                                bleParam[bleParamCnt - 1][bleParamCharCnt[bleParamCnt - 1]] = char(c);
                                bleParamCharCnt[bleParamCnt - 1]++;
                            }
                        }
                    }
                    else{
                        // Delimiter received or buffer limit met
                        rxWord[cmdWordCnt] = '\0'; // NULL terminator
                    }
                }
            }
        }

        // Convert BLE Parameters to integers
        if (bleParamFound){
            for(int i = 0; i < bleParamCnt; i++){
                bleParam[i][bleParamCharCnt[i]] = '\0';
                bleParamInt[i] = atoi(bleParam[i]);
            }

        }

        if (!strncmp(rxCmd, cmdSearch, sizeof(cmdSearch))){
            // Send preamble over BLE, return data should be comma delimited and terminated with a semicolon
            String valToSend = ">";
            valToSend += cmdSearch;
            valToSend += ",";
            int length = valToSend.length() + 1;
            char charBuf[length];
            valToSend.toCharArray(charBuf, length);
            ble.println(charBuf);

            int ble_exit_detect = searchForMField(bleParamInt[0], bleParamInt[1], bleParamInt[2]);

            // Terminate string
            if (!ble_exit_detect){
                ble.println(";");
            }

        }

        // if (!strncmp(rxCmd, cmdStart, sizeof(cmdStart))){
            // loopDSP = bleParamInt[0];
            // excecuteDSP = 1;
            // ble.print(F(">")); // Acknowledge rx cmd
            // ble.print(F(cmdStart));
        // }

        if (!strncmp(rxCmd, cmdCal, sizeof(cmdCal))){
            // Send preamble over BLE, return data should be comma delimited and terminated with a semicolon
            String valToSend = ">";
            valToSend += cmdCal;
            int length = valToSend.length() + 1;
            char charBuf[length];
            valToSend.toCharArray(charBuf, length);
            ble.println(charBuf);

            runCal(bleParamInt[0]);

            // Terminate string
            ble.println(";");

        }

        if (!strncmp(rxCmd, cmdThresh, sizeof(cmdThresh))){
            // Send preamble over BLE, return data should be comma delimited and terminated with a semicolon
            String valToSend = ">";
            valToSend += cmdThresh;
            valToSend += ",";
            int length = valToSend.length() + 1;
            char charBuf[length];
            valToSend.toCharArray(charBuf, length);
            ble.println(charBuf);

            setGetThresh(bleParamInt[0], bleParamInt[1]);

            // Terminate string
            ble.println(";");

        }

        if (!strncmp(rxCmd, cmdBattVolt, sizeof(cmdBattVolt))){
            // Send preamble over BLE, return data should be comma delimited and terminated with a semicolon
            String valToSend = ">";
            valToSend += cmdBattVolt;
            valToSend += ",";
            int length = valToSend.length() + 1;
            char charBuf[length];
            valToSend.toCharArray(charBuf, length);
            ble.println(charBuf);

            getBattVolts();

            // Terminate string
            ble.println(";");

        }

    }


    /* **********************************
        Start LED process
       ******************************** */

//  // set the brightness of pin 9:
//  analogWrite(led, brightness);
//
//  // change the brightness for next time through the loop:
//  brightness = brightness + fadeAmount;
//
//  // reverse the direction of the fading at the ends of the fade:
//  if (brightness <= 0 || brightness >= 255) {
//      fadeAmount = -fadeAmount;
//  }
//  // wait for 30 milliseconds to see the dimming effect
//  // delay(30);
    for(int i = 0; i < loopDSP; i++){
        /* **********************************
            Get AOS sensor data prior
                to the ADC collection
        ******************************** */
        // Get a new sensor event
        bno.getEvent(&event);

        // Get AOS Y-axis angle
        aosAngleOffs = event.orientation.y;
        // aosAngleOffs = 0.0; // Test without AOS sensor

        /* **********************************
            Start ADC process
        ******************************** */
        // read the analog in values
        // getAdcData(0);

        // Convert the raw values to voltages and store them in adcVolt[all][3]
        // convertToVolts(0);

        /* **********************************
            ADC Low pass filter
        ******************************** */
        filterVoltsLP(100);

        /* **********************************
            Find max or min value
        ******************************** */
        int m = findPeak();
        int peakType = getPeakType(m); // Indicates type of peak: none, max, min

        /* **********************************
            Pre-process the data prior to the Curve fitting
        ******************************** */
        int boundaryXed = CF_dataInit(m);

        // Normalize the magnitudes to keep numbers small
        // NOTE: This step may not be necessary due to sensor limits being 0V - 2V
        // normCF_Data();

        /* **********************************
            Run Curve fit (CF) algorithm and get angle
        ******************************** */
        float maxCFVal = 0.0;
        if (peakType != 0){
            maxCFVal = getAngle(boundaryXed, peakType);
        }

        /* **********************************
            Debug output to serial
        ******************************** */
        // This section of code is for debuggin only

        Serial.print(maxCFVal);
        Serial.print(", ");
        Serial.print(m);
        Serial.println("");

        // The BLE device has a problem with multiple back-to-back writes
        // Concatenate a string and send it as one write process.
        String valToSend = String(maxCFVal);
        valToSend += ", ";
        valToSend += m;
        char charBuf[20];
        valToSend.toCharArray(charBuf, 20);
        ble.println(charBuf);

        /* Get a new sensor event */
        // bno.getEvent(&event);
        /* Display the floating point data */
        // Serial.print("X: ");
        // Serial.print(event.orientation.x, 4);
        // Serial.print("\tY: ");
        // Serial.print(event.orientation.y, 4);
        // Serial.print("\tZ: ");
        // Serial.print(event.orientation.z, 4);
        // Serial.println("");

        // wait delay(<num>) milliseconds before the program loops again
        delay(200);

    }
}


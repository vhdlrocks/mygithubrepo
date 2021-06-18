/*
Feather 32u4:
32k flash
2k RAM
ATmega32u4 @ 8MHz with 3.3V logic/power
3.3V regulator with 500mA peak current output

Notes:
    This was written for the Feather 32u4. The M0 version doesn't have EEPROM.

M0 configuration:
    User changes have been commented using tags of the format <tag>. To make
    changes I've included these tags to help indicate which pieces of code need
    to be modified to make additional changes. In some cases a change has to be
    made in multiple areas. Search the <tag> in all files to find them.

    <EEPROM> :
        M0 doesn't have EEPROM, only the Feather 32u4. Comment out
        EEPROM functions (should already be done).
    <PINOUT> :
        Modify these pins and their settings based on your needs.
    <TEST> :
        To modify or add tests, make changes here
    <CHANGE VAR DEFAULTS> :
        Since the M0 doesn't have EEPROM, you can change the system variable
        default values here and recompile/upload so you don't have to run
        >> vars set var_num var_val every time your system is reset.
    <SYSTEM VARIABLES> :
        These variables are used to configure waveforms generated
        by a specific test. Currently there is only one test, RUN_TEST_NUM 1.

Adding additional tests:
*/

// <EEPROM>
#include <EEPROM.h>

struct AFRLObject {
    int CAM_A_ON;
    int CAM_B_ON;
    int RLY_0_ON;
    int RLY_0_OFF;
    int CAM_A_OFF;
    int CAM_B_OFF;
    int LED_RATE_SLOW;
    int LED_RATE_FAST;
};

// <PINOUT> : Define Outputs
#define AFRL_RLY_0   2 // Control for relay 0
// #define AFRL_RLY_1   3 // Control for relay 1
// #define AFRL_RLY_2   5 // Control for relay 2
#define AFRL_CAM_A  16 // Control for camera A
#define AFRL_CAM_B  14 // Control for camera B


// <CHANGE VAR DEFAULTS>, <SYSTEM VARIABLES> User variables
volatile int CAM_A_ON       = 2000; // Time sequence to turn camera ON, set to 0 to disable
volatile int CAM_B_ON       = 2000; // Time sequence to turn camera ON, set to 0 to disable
volatile int RLY_0_ON       = 3000; // Time sequence to turn relay ON, set to 0 to disable
volatile int RLY_0_OFF      = 4000; // Time sequence to turn relay OFF
volatile int CAM_A_OFF      = 7000; // Time sequence to turn camera OFF
volatile int CAM_B_OFF      = 7000; // Time sequence to turn camera OFF
volatile int LED_RATE_SLOW  = 400;  // LED Blink rate after relay is turned off
volatile int LED_RATE_FAST  = 100;  // LED Blink rate before relay is turned on

// Test global variables
volatile bool RUN_ACTIVE = false;   // Indicates run status
volatile uint8_t RUN_TEST_NUM;      // Test to run

volatile unsigned long testTimeKeeper = 0; // Test start time

const bool eeDflt = true;

void setup() {


    Serial.begin(115200);
    while (!Serial);
    Serial.println(F("Initializing AFRL peripherals"));

    // <EEPROM> : No EEPROM on M0!!!
    // if (eeDflt) {
    //     Serial.print(F("Resetting non-vol memory to default"));
    //     eepWrite();
    //     Serial.println(F("..OK"));
    // } else {
    //     Serial.print("Updating vars from non-vol memory");
    //     eepRead(false);
    //     Serial.println(F("..OK"));
    // }

    // <PINOUT> : Set direction and default logic level
    pinMode(AFRL_RLY_0, OUTPUT);
    // pinMode(AFRL_RLY_1, OUTPUT);
    // pinMode(AFRL_RLY_2, OUTPUT);
    pinMode(AFRL_CAM_A, OUTPUT);
    pinMode(AFRL_CAM_B, OUTPUT);
    digitalWrite(AFRL_RLY_0, LOW);
    // digitalWrite(AFRL_RLY_1, LOW);
    // digitalWrite(AFRL_RLY_2, LOW);
    digitalWrite(AFRL_CAM_A, LOW);
    digitalWrite(AFRL_CAM_B, LOW);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

}

bool stateLast;

void loop() {
    fetchUart();
    // afrl
    if (RUN_ACTIVE) {
        if (stateLast != RUN_ACTIVE) {
            Serial.println("Active");
            stateLast = RUN_ACTIVE;
        }
        // <TEST>
        runAFRL_test1(getTestFinalTime());
    } else {
        if (stateLast != RUN_ACTIVE) {
            Serial.println("Inactive");
            stateLast = RUN_ACTIVE;
        }
        digitalWrite(AFRL_RLY_0, LOW);
        // digitalWrite(AFRL_RLY_1, LOW);
        // digitalWrite(AFRL_RLY_2, LOW);
        digitalWrite(AFRL_CAM_A, LOW);
        digitalWrite(AFRL_CAM_B, LOW);
        digitalWrite(LED_BUILTIN, LOW);
    }

}


// <TEST>, <SYSTEM VARIABLES>
/*
    Test requirements:
        turn on camera
        led flash
        fire
        turn off camera
*/
volatile uint8_t ledState = 0;
volatile uint8_t ledStateLast = 3; //debug
volatile unsigned long ledTimeLast = 0;

void runAFRL_test1(int thisMaxTime) {
    // initialize time keeper
    if (testTimeKeeper == 0) testTimeKeeper = millis();

    unsigned long currentTime = millis();

    // LED
    switch(ledState) {
        case 0:
            if (currentTime - ledTimeLast > LED_RATE_FAST) {
                ledTimeLast = currentTime;
                digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            }
            break;

        case 1:
            digitalWrite(LED_BUILTIN, HIGH);
            break;

        case 2:
            if (currentTime - ledTimeLast > LED_RATE_SLOW) {
                ledTimeLast = currentTime;
                digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            }
            break;

    }

    if (ledState != ledStateLast) {
        // Serial.print("LED State: "); Serial.println(ledState);
        showState(ledState);
        ledStateLast = ledState;
    }

    // Camera A
    if (currentTime - testTimeKeeper > CAM_A_OFF) {
        digitalWrite(AFRL_CAM_A, LOW);
    } else if (currentTime - testTimeKeeper > CAM_A_ON) {
        digitalWrite(AFRL_CAM_A, HIGH);
    }

    // Camera B
    if (currentTime - testTimeKeeper > CAM_B_OFF) {
        digitalWrite(AFRL_CAM_B, LOW);
    } else if (currentTime - testTimeKeeper > CAM_B_ON) {
        digitalWrite(AFRL_CAM_B, HIGH);
    }

    // Relay
    if (currentTime - testTimeKeeper > RLY_0_OFF) {
        digitalWrite(AFRL_RLY_0, LOW);
        ledState = 2;
    } else if (currentTime - testTimeKeeper > RLY_0_ON) {
        digitalWrite(AFRL_RLY_0, HIGH);
        ledState = 1;
    }

    // Reset test vars
    if (currentTime - testTimeKeeper > thisMaxTime) {
        RUN_ACTIVE = false;
        ledState = 0;
        ledTimeLast = 0;
        testTimeKeeper = 0;
        ledStateLast = 3;
        showState(ledStateLast);
    }


}

// <SYSTEM VARIABLES>
int getTestFinalTime() {
    int maxTime = 0;
    if (CAM_A_ON  > maxTime) maxTime = CAM_A_ON ;
    if (CAM_B_ON  > maxTime) maxTime = CAM_B_ON ;
    if (RLY_0_ON  > maxTime) maxTime = RLY_0_ON ;
    if (RLY_0_OFF > maxTime) maxTime = RLY_0_OFF;
    if (CAM_A_OFF > maxTime) maxTime = CAM_A_OFF;
    if (CAM_B_OFF > maxTime) maxTime = CAM_B_OFF;
    return maxTime + 1000;
}

void showState(uint8_t state) {
    switch(state) {
        case 0:
            Serial.println("Waiting for trigger...");
            break;
        case 1:
            Serial.println("Triggered relay...");
            break;
        case 2:
            Serial.println("Waiting until done...");
            break;
        case 3:
            Serial.println("Test complete");
            break;
            
    }
}
/*
CSM system requirements:
1. Z-axis control (stepper motor, 0.9 deg per step max)
2. Mill (BLDC motor, 4000 RPM optimal, max can be higher but not required)
3. User interface (OLED display, push buttons)
4. Non-vol memory for settings/configurations (SD Card)
5. Power supplies and related Support hardware
    - Power supply
    - Enclosure (provided by mechanical team)

CSM User interface requirements:
IF menu:
    Drill modes:
        Manual mode
            (Display output)
                Running manual:
                Joystick for analog ctrl
                Buttons for incremental ctrl
                Press "select" when ready
                    (Display output)
                        Z-axis pos: <mm>
                        Drill RPM: <rpm>
                        Press "select" to exit
        Automatic mode
            Use current config
                Run
            Load config
            Make new config
                New profile ID: <hole_id>
                    constant
                        mm/min: <num_mm_per_min>
                            Rotation: <cw/ccw>
                    peck
                        mm/min: <num_mm_per_min>
                            pecks per cnt ((mm/min)/cnt): <cnt>
                                Rotation: <cw/ccw>
                    custom
                        **Need to define**
    Global Config:
        Max drill depth (mm): <max_mm>
        Min drill RPM (rpm): <min_rpm>
        Max drill RPM (rpm): <max_rpm>
        Default hole ID (if available): <dft_hole_id>

CSM functional requirements:

IO assignments:
    Available
        Analog: 18(A0), 19(A1), 20(A2), 21(A3), 22(A4), 23(A5)
        Digital: 0/RX, 1/TX, 2/SDA, 3/SCL, 5, 6, 9, 10, 11, 12, 13, 15/SCLK(SD), 16/MOSI(SD), 14/MISO(SD)

    Assigned:
        Display: 2/SDA, 3/SCL
        SD Card: shared SPI bus: SCK, MOSI, MISO
        Stepper: 12, 13, 22(A4), 23(A5)
        Brushless motor: 0, 6, 9, 10, 11, 18(A0), 20(A2)


Notes about Adalogger 32u4:
    clock frequency: 8 MHz
    Interrupt pins: 0, 1, 2, 3, 7 (pin 7 interrupt is not available, used by SD)

Original arduino.json
{
    "board": "arduino:avr:leonardo",
    "programmer": "ArduinoISP",
    "port": "COM22",
    "prebuild": "./prebuild.sh",
    "postbuild": "./postbuild.sh",
    "intelliSenseGen": "global"
}

Types:          Size in Bytes           Contains
boolean             1                   True (1), False (0)
char                1                   -128 to 127
unsigned char, 
byte, uint8_t       1                   0 to 255
int, short          2                   -32,768 to 32,767
unsigned int, word,
uint16_t            2                   0 to 65,535
long                4                   -2,147,483,648 to 2,147,483,647
unsigned long
uint32_t            4                   0 to 4,294,967,295
float, double       4                   -3.4028235e38 to 3.4028235e38

*/

#include <SPI.h>
#include <Wire.h>               // graphics, joystick
#include <Adafruit_GFX.h>       // graphics
#include <Adafruit_SSD1306.h>   // graphics
#include <SD.h>

/*--------------------------------
        SD Card
   -------------------------------*/
File root;

// Feather 32u4 clock frequency
// #define CPU_HZ 8000000 // 8 MHz


/*--------------------------------
        Display
   -------------------------------*/
// Initialize OLED display
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

/*--------------------------------
        IO placement
   -------------------------------*/
//Define Outputs
#define BM_FWD          18 // was 8 on leo
#define BM_REV          9
// #define BM_STOP_MODE    10 // Tie to gnd, Leave logic pin high (or invert in sw and tie to ground) on controller (prevents instantaneous stops)
// #define BM_M0           11 // Tie to gnd, Can remove and leave logic pin low (setting #1, tie low)
// #define BM_M1           12 // Tie to gnd, Can remove and use analog input on setting #1
#define BM_ALM_RST      10
#define BM_SPD_OUT      11 // PWM output, was 5 on leo
#define BM_TIMER_TEST   20 // 1 second interrupt test pin, was 4 on leo

//Define Inputs
#define BM_SPD_IN       0 // Was 7 on leo, needs alternate interrupt pin
#define BM_ALM          6 // **Not currently being used**

// Output related to brushless motor driver (BLE2D60-A)
#define BM_ACTIVE       0
#define BM_INACTIVE     1

// Display digitial IO (not using)
// #define DISPLAY_BTN_A 9
// #define DISPLAY_BTN_B 6
// #define DISPLAY_BTN_C 5

// Stepper digital IO
#define SM_PLS          13
#define SM_DIR          12
// #define SM_AWO 11 // Tie to gnd, Should never have to turn this on ('1' turns off winding current)
// #define SM_CS  10 // Tie to gnd, Should be able to tie this pin low on unit ('0' uses angle set on switch)
#define SM_ALM 22 // DIO 22
#define SM_TIM 23 // DIO 23

#define HOME_DET A1 // DIO 19
uint8_t sys_timer_epoch_done = 0; // Set by ISR(TIMER1_OVF_vect), reset by getRPM()



int SM_STEP_POS = 0; // Stepper current step from home
volatile float RPM = 0; // Calculated RPM
#define BM_DIR_FWD 1;
#define BM_DIR_REV 0;
uint8_t BM_DIR = BM_DIR_REV;


#define BUTTON_RIGHT 6
#define BUTTON_DOWN  7
#define BUTTON_LEFT  9
#define BUTTON_UP    10
#define BUTTON_SEL   14
#define JOYSTICK_VAL 15
#define BUTTON_DFLT  0

uint32_t button_mask = (1 << BUTTON_RIGHT) | (1 << BUTTON_DOWN) | (1 << BUTTON_LEFT) | (1 << BUTTON_UP) | (1 << BUTTON_SEL);




int timer1_counter; // 1-sec timer preset

void init_sd_card(void);
void init_display(void);
void init_display(void);
void init_js(void);
void init_bm(void);
void init_sm(void);

void setup() {


    Serial.begin(115200);
    // while (!Serial);

    Serial.println(F("[CSM] Initializing peripherals"));

    // init_sd_card();
    // while(1);
    init_display();
    init_js();
    init_bm();
    init_sm();
}

int js_val = 0;
int js_new_val = 0;

int lastStepPos, lastRPM = 0;
int lastDir = 1;

// Value of joytick offsets
int js_x, js_y = 511;


long sysLoopLastTime = 0; // Last loop time for system
long jsPollRate = 50000;

void loop() {


    // Run 1-second process updates
    if (sys_timer_epoch_done == 1) {
        sys_timer_epoch_done = 0;
        getRPM();
        // digitalWrite(BM_TIMER_TEST, digitalRead(BM_TIMER_TEST) ^ 1);
        // Serial.print(SM_STEP_POS);  Serial.print(" ");  Serial.print((int)RPM);   Serial.print(" ");  Serial.print(BM_DIR); 
        if ((lastStepPos != SM_STEP_POS) || (lastRPM != (int)RPM) || (lastDir != BM_DIR)){
            show_mtr_basic(SM_STEP_POS, (int)RPM, BM_DIR);
            lastStepPos = SM_STEP_POS;
            lastRPM = (int)RPM;
            lastDir = BM_DIR;
        }
    } else {

        long currentTime = micros();
        if ((currentTime - sysLoopLastTime) > jsPollRate) {
            // Serial.print("Time: "); Serial.print(currentTime); Serial.print("sysLoopLastTime: "); Serial.print(sysLoopLastTime); Serial.print("Delta: "); Serial.println(currentTime - sysLoopLastTime); 
            sysLoopLastTime = currentTime;
            // Poll joystick events
            js_new_val = js_update(&js_x, &js_y);

            // Execute functions based on joystick inputs
            if (js_new_val != 0) {
                int x_val, y_val = 0;
                if (js_new_val == JOYSTICK_VAL) setAnalogConv(js_x, js_y, &x_val, &y_val);
                sm_update(js_new_val, x_val);
                bm_update(js_new_val, y_val);
            }

            bm_run();

        }


    }

}



void setAnalogConv(int update_x, int update_y, int* x_conv, int* y_conv){

    update_x = 511 - update_x;
    if (update_x < 0) {
        *x_conv = -1*sq(update_x/64); // Square root
    } else {
        *x_conv = sq(update_x/64); // Square root
    }
    update_y = 511 - update_y;
    *y_conv = (update_y/64);

}



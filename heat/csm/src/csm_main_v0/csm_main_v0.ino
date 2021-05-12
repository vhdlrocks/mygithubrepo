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
        Analog: A0, A1, A2, A3, A4, A5
        Digital: 0/RX, 1/TX, 2/SDA, 3/SCL, 4, 6, 9, 10, 11, 12, 13, SCK, MOSI, MISO
    
    Assigned:
        Display: 5, 6, 9, 2/SDA, 3/SCL
        SD Card: shared SPI bus: SCK, MOSI, MISO
        Stepper: 10, 11, 12, 13, A4, A5
*/

#include <SPI.h>
#include <Wire.h>               // graphics, joystick
#include <Adafruit_GFX.h>       // graphics
#include <Adafruit_SSD1306.h>   // graphics


// Feather 32u4 clock frequency
#define CPU_HZ 8000000 // 8 MHz


// Initialize OLED display
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

/*--------------------------------
        IO placement
   -------------------------------*/
//Define Outputs
#define BM_FWD         18 // was 8 on leo
#define BM_REV         9
// #define BM_STOP_MODE   10 // Tie to gnd, Leave logic pin high (or invert in sw and tie to ground) on controller (prevents instantaneous stops)
// #define BM_M0          11 // Tie to gnd, Can remove and leave logic pin low (setting #1, tie low)
// #define BM_M1          12 // Tie to gnd, Can remove and use analog input on setting #1
#define BM_ALM_RST     10
#define BM_SPD_OUT  11 // PWM output, was 5 on leo
#define BM_TIMER_TEST  20 // 1 second interrupt test pin, was 4 on leo

//Define Inputs
#define BM_SPD_IN    0 // Was 7 on leo, needs alternate interrupt pin
#define BM_ALM      6 // **Not currently being used**

// Output related to brushless motor driver (BLE2D60-A)
#define BM_ACTIVE 0
#define BM_INACTIVE 1

// Display digitial IO
// #define DISPLAY_BTN_A 9
// #define DISPLAY_BTN_B 6
// #define DISPLAY_BTN_C 5

// Stepper digital IO
#define SM_PLS          13
#define SM_DIR          12
// #define SM_AWO 11 // Tie to gnd, Should never have to turn this on ('1' turns off winding current)
// #define SM_CS  10 // Tie to gnd, Should be able to tie this pin low on unit ('0' uses angle set on switch)
#define SM_ALM A4 // DIO 22
#define SM_TIM A5 // DIO 23

void setup() {
    Serial.begin(115200);

    Serial.println("OLED FeatherWing test");
//   // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
//   display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

//   Serial.println("OLED begun");

//   // Show image buffer on the display hardware.
//   // Since the buffer is intialized with an Adafruit splashscreen
//   // internally, this will display the splashscreen.
//   display.display();
//   delay(1000);

//   // Clear the buffer.
//   display.clearDisplay();
//   display.display();

    Serial.println("IO test");


    init_display();
    init_js();
    init_sm();
}

int js_val = 0;
int js_new_val = 0;

void loop() {

    // Poll joystick events
    js_new_val = js_update();

    // Execute functions based on joystick inputs
    if (js_new_val != 0) {
        sm_update(js_new_val);
    }
    // if(!digitalRead(DISPLAY_BTN_A)) display.print("A");
    // if(!digitalRead(DISPLAY_BTN_B)) display.print("B");
    // if(!digitalRead(DISPLAY_BTN_C)) display.print("C");

    // Update display
    delay(10);
    display.display();
    bm_run();
    
}







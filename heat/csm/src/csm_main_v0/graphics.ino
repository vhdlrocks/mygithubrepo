


// Notes: setTextSize(1) 21 chars per line, 4 lines
// Notes: setTextSize(2) 10 chars per line, 2 lines

// Adafruit_SSD1306 graphics driver

/* ----------------------------------------------
    init_aos : Initialize the AOS (Absolute Orientation Sensor)
-----------------------------------------------*/
// No references (global reference only: bno, Serial, delay)
void init_display(){

    // while (!Serial);
    /* Initialise the sensor */
    Serial.print(F("Initializing OLED Display..."));
    // Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

    Serial.println(F("OK"));

    // pinMode(DISPLAY_BTN_A, INPUT_PULLUP);
    // pinMode(DISPLAY_BTN_B, INPUT_PULLUP);
    // pinMode(DISPLAY_BTN_C, INPUT_PULLUP);

    // Show image buffer on the display hardware.
    // Since the buffer is intialized with an Adafruit splashscreen
    // internally, this will display the splashscreen.
    // display.display();
    // delay(1000);

    // Clear the buffer.
    display.clearDisplay();
    display.display();

    // text display tests
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    // display.println("Welcome to CSM!");
    // display.println("Let's drill down to buisness!");
    display.println("Button Control DEMO:");
    display.println("Stepper Motor: UP/DWN");
    display.println("Select: Return Home");
    display.println("Drill Motor: LFT/RGT");
    // int16_t xval = display.getCursorX();
    // int16_t yval = display.getCursorY();
    // Serial.print("X: ");
    // Serial.print(xval);
    // Serial.print(" Y: ");
    // Serial.print(yval);
    // Serial.println("");
    // display.setCursor(0,0);
    // xval = display.getCursorX();
    // yval = display.getCursorY();
    // Serial.print("X: ");
    // Serial.print(xval);
    // Serial.print(" Y: ");
    // Serial.print(yval);
    // Serial.println("");

    display.display(); // actually display all of the above
    delay(1000);

    // if(!bno.begin())
    // {
    //     /* There was a problem detecting the BNO055 ... check your connections */
    //     Serial.println("FAILURE");
    //     while(1);
    // }
    // else{
    //     Serial.println("PASSED");
    // }

    // delay(1000);

    // bno.setExtCrystalUse(true);
}

//Only clear the display the first time this screen is shown
uint8_t displayFirstTime = 1;

void show_mtr_basic(int d_step, int d_rpm, uint8_t d_dir){

    String s_sm_label = "Step";
    String s_bm_label = "Dir/RPM";
    String s_bm_fwd = ">";
    String s_bm_rev = "<";
    String s_spacer = "---------------------";
    String s_bm_dir = s_bm_fwd;

    if (d_dir == 1) {
        s_bm_dir = s_bm_fwd;
    } else {
        s_bm_dir = s_bm_rev;
    }

    // if (displayFirstTime == 1) {
    //     displayFirstTime = 0;
        // Clear the buffer.
        display.clearDisplay();
        display.display();
    // }

        // text display tests
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0,0);
        // display.println("Welcome to CSM!");
        // display.println("Let's drill down to buisness!");
        display.println("    Step     RPM");
        display.println(s_spacer);
        display.setTextSize(2);
    // }

    // int16_t xval = display.getCursorX();
    // int16_t yval = display.getCursorY();
    // Serial.print("X: ");
    // Serial.print(xval);
    // Serial.print(" Y: ");
    // Serial.print(yval);
    // Serial.println("");
    // while(1);


    // Start at 0, 16
    // display.setCursor(0,16);

    //5 digits allocated for step, 1 for spacer, 4 for RPM
    // display.println("         ");
    // display.display(); // actually display all of the above
    // delay(1);

    display.setCursor(0,16);
    display.print(d_step);
    display.print(" ");
    display.print(s_bm_dir);
    display.println(d_rpm);
    // display.print(" ");
    // display.print(s_bm_dir);
    // display.println();
    // int16_t xval = display.getCursorX();
    // int16_t yval = display.getCursorY();
    // Serial.print("X: ");
    // Serial.print(xval);
    // Serial.print(" Y: ");
    // Serial.print(yval);
    // Serial.println("");
    // display.setCursor(0,0);
    // xval = display.getCursorX();
    // yval = display.getCursorY();
    // Serial.print("X: ");
    // Serial.print(xval);
    // Serial.print(" Y: ");
    // Serial.print(yval);
    // Serial.println("");

    display.display(); // actually display all of the above
    delay(1);
    sysLoopLastTime = micros() + 90000; // Measured 72 ms

}




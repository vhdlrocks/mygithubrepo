




// Adafruit_SSD1306 graphics driver

/* ----------------------------------------------
    init_aos : Initialize the AOS (Absolute Orientation Sensor)
-----------------------------------------------*/
// No references (global reference only: bno, Serial, delay)
void init_display(){
    /* Initialise the sensor */
    Serial.print("Initializing OLED Display...");
    // Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

    Serial.println("OK");

    // pinMode(DISPLAY_BTN_A, INPUT_PULLUP);
    // pinMode(DISPLAY_BTN_B, INPUT_PULLUP);
    // pinMode(DISPLAY_BTN_C, INPUT_PULLUP);

    // Show image buffer on the display hardware.
    // Since the buffer is intialized with an Adafruit splashscreen
    // internally, this will display the splashscreen.
    display.display();
    delay(1000);

    // Clear the buffer.
    display.clearDisplay();
    display.display();

    // text display tests
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("Welcome to CSM!");
    display.println("Let's drill down to buisness!");
    display.setCursor(0,0);
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

//void drawPixel(uint16_t x, uint16_t y, uint16_t color);





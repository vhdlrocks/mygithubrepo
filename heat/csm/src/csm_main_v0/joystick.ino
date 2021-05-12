

#include "Adafruit_seesaw.h"

Adafruit_seesaw ss;

#define BUTTON_RIGHT 6
#define BUTTON_DOWN  7
#define BUTTON_LEFT  9
#define BUTTON_UP    10
#define BUTTON_SEL   14
#define BUTTON_DFLT  0

uint32_t button_mask = (1 << BUTTON_RIGHT) | (1 << BUTTON_DOWN) | (1 << BUTTON_LEFT) | (1 << BUTTON_UP) | (1 << BUTTON_SEL);

#define IRQ_PIN 5

void init_js() {

    // Serial.println("Joy FeatherWing example!");
    /* Initialise the sensor */
    Serial.print("Initializing HMI...");

    if(!ss.begin(0x49)){
        Serial.println("ERROR! seesaw not found");
        while(1);
    } else {
        Serial.print("OK");
        Serial.print(" Seesaw driver version: ");
        Serial.println(ss.getVersion(), HEX);
    }
    ss.pinModeBulk(button_mask, INPUT_PULLUP);
    ss.setGPIOInterrupts(button_mask, 1);

    // pinMode(IRQ_PIN, INPUT);
}


int last_x = 0, last_y = 0;

int js_update() {
    int x = ss.analogRead(2);
    int y = ss.analogRead(3);

    if ( (abs(x - last_x) > 3)  ||  (abs(y - last_y) > 3)) {
        Serial.print(x); Serial.print(", "); Serial.println(y);
        last_x = x;
        last_y = y;
    }
  
    /* if(!digitalRead(IRQ_PIN)) {  // Uncomment to use IRQ */

    uint32_t buttons = ss.digitalReadBulk(button_mask);

    //Serial.println(buttons, BIN);

    if (! (buttons & (1 << BUTTON_RIGHT))) {
      Serial.println("Button A pressed");
      return BUTTON_RIGHT;
    }
    if (! (buttons & (1 << BUTTON_DOWN))) {
      Serial.println("Button B pressed");
      return BUTTON_DOWN;
    }
    if (! (buttons & (1 << BUTTON_LEFT))) {
      Serial.println("Button Y pressed");
      return BUTTON_LEFT;
    }
    if (! (buttons & (1 << BUTTON_UP))) {
      Serial.println("Button X pressed");
      return BUTTON_UP;
    }
    if (! (buttons & (1 << BUTTON_SEL))) {
      Serial.println("Button SEL pressed");
      return BUTTON_SEL;
    }
    /* } // Uncomment to use IRQ */
    delay(10);
    return BUTTON_DFLT;
}
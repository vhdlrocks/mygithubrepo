



void init_sm() {

    Serial.print(F("Initializing Stepper Driver CVD228BF-K..."));

    pinMode(SM_PLS, OUTPUT);
    pinMode(SM_DIR, OUTPUT);
    // pinMode(SM_AWO, OUTPUT);
    // pinMode(SM_CS, OUTPUT);
    pinMode(SM_ALM, INPUT_PULLUP);
    pinMode(SM_TIM, INPUT_PULLUP);

    digitalWrite(SM_PLS, LOW);
    digitalWrite(SM_DIR, LOW);
    // digitalWrite(SM_AWO, LOW);
    // digitalWrite(SM_CS,  LOW);

    Serial.println(F("OK"));

}


#define CW 1
#define CCW 0


// int global_dir = CW;

void sm_update(int sm_cmd, int updateAnalogStep) {

    switch(sm_cmd) {

        case BUTTON_UP: // Single step
            digitalWrite(SM_DIR, CW);
            sm_take_step(1, CW);
            break;

        case BUTTON_DOWN: // Single step
            digitalWrite(SM_DIR, CCW);
            sm_take_step(1, CCW);
            break;

        case BUTTON_SEL: // Go home
            if (SM_STEP_POS > 0) {
                digitalWrite(SM_DIR, CCW);
                sm_take_step(SM_STEP_POS, CCW);
            } else if (SM_STEP_POS < 0) {
                digitalWrite(SM_DIR, CW);
                sm_take_step(-1*SM_STEP_POS, CW);
            }
            break;

        case JOYSTICK_VAL: // Variable step
            if (updateAnalogStep > 0) {
                digitalWrite(SM_DIR, CW);
                sm_take_step(updateAnalogStep, CW);
            } else if (updateAnalogStep < 0) {
                digitalWrite(SM_DIR, CCW);
                sm_take_step(-1*updateAnalogStep, CCW);
            }
            break;

    }
}


void sm_take_step(int num_steps, int direction) {
    int i;
    for(i = 0; i<num_steps; i++) {
        digitalWrite(SM_PLS, HIGH);
        delayMicroseconds(200);
        digitalWrite(SM_PLS, LOW);
        delayMicroseconds(300);
        if (direction == CW) {
            SM_STEP_POS = SM_STEP_POS + 1;
        } else {
            SM_STEP_POS = SM_STEP_POS - 1;
        }
        // Serial.println(SM_STEP_POS);

    }
}

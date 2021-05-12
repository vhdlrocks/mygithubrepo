




void init_sm() {

    Serial.print("Initializing Stepper Driver CVD228BF-K...");

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

    Serial.println("OK");

}


#define CW 1
#define CCW 0

#define FULL_STEP 1
#define HALF_STEP 0

// int global_dir = CW;

void sm_update(int sm_cmd) {
    // unsigned long timeNowUS = micros();

    // if (sm_cmd == BUTTON_UP) {
    //     digitalWrite(SM_PLS, HIGH);
    //     delayMicroseconds(100);
    //     digitalWrite(SM_PLS, LOW);
    // } else {

    switch(sm_cmd) {

        case BUTTON_UP:
            digitalWrite(SM_DIR, CW);
            // digitalWrite(SM_CS, FULL_STEP);
            sm_step();
            break;

        case BUTTON_DOWN:
            digitalWrite(SM_DIR, CCW);
            // digitalWrite(SM_CS, HALF_STEP);
            sm_step();
            break;


        // case BUTTON_LEFT:
        //     digitalWrite(SM_PLS, HIGH);
        //     delayMicroseconds(10);
        //     digitalWrite(SM_PLS, LOW);
        //     break;

        // case BUTTON_RIGHT:
        //     digitalWrite(SM_PLS, HIGH);
        //     delayMicroseconds(10);
        //     digitalWrite(SM_PLS, LOW);
        //     break;

    }

}


void sm_step() {
    digitalWrite(SM_PLS, HIGH);
    delayMicroseconds(10);
    digitalWrite(SM_PLS, LOW);
}






/*--------------------------------
        getSerial() global variables/constants
   -------------------------------*/
#define sub_cmds 10
#define cmd_buff_size 20
String inputCmd[sub_cmds] = ""; // a String to hold incoming data
bool inputCmdComplete = false;  // whether the string is complete
int sub_cmd_idx = 0;


/*--------------------------------
        getRPM(), printRPM() global variables/constants
   -------------------------------*/
volatile float RPM = 0; // Calculated RPM
#define DUTY2PWM(x)  ((255*(x))/100) // Macro to convert from duty (0..100) to PWM (0..255)
int dutyCycle = 0; // PWWM duty cycle used to program timer, 0..255



/*--------------------------------
        encISR_A(), ISR(TIMER1_OVF_vect) global variables/constants
   -------------------------------*/
int fract_rotation_cnt = 0; // Counts the number pulses in a given TIMER1 interval
int last_rotation_cnt = 0;  // Last fract_rotation_cnt to calculate new RPM value
uint8_t int_timer_epoch_done = 0; // Set by ISR(TIMER1_OVF_vect), reset by encISR_A()
uint8_t sys_timer_epoch_done = 0; // Set by ISR(TIMER1_OVF_vect), reset by getRPM()
int timer1_counter; // 1-sec timer preset
#define timer1_counter 34286 // preload timer 65536-16MHz/256/2Hz








/**
 * 
 * From BM driver test
 * 
 * 
 * 
 * 
 * 
 */

/**
 * System setup
 */
void bm_setup() {

    Serial.print("Initializing Brushless Motor (BM) driver...");

    // initialize timer1 
    noInterrupts();           // disable all interrupts
    timer_1sec_int_init();
    interrupts();             // enable all interrupts

    Serial.begin(115200);

    pinMode(BM_FWD         , OUTPUT);
    pinMode(BM_REV         , OUTPUT);
    // pinMode(BM_STOP_MODE      , OUTPUT);
    // pinMode(BM_M0          , OUTPUT);
    // pinMode(BM_M1          , OUTPUT);
    pinMode(BM_ALM_RST     , OUTPUT);
    pinMode(BM_SPD_OUT  , OUTPUT);
    pinMode(BM_TIMER_TEST  , OUTPUT);


    pinMode(BM_ALM      , INPUT);
    pinMode(BM_SPD_IN    , INPUT);

    digitalWrite(BM_FWD        , BM_INACTIVE);
    digitalWrite(BM_REV        , BM_INACTIVE);
    // digitalWrite(BM_STOP_MODE  , BM_INACTIVE);
    // digitalWrite(BM_M0         , BM_INACTIVE);
    // digitalWrite(BM_M1         , BM_INACTIVE);
    digitalWrite(BM_ALM_RST    , BM_INACTIVE);
    digitalWrite(BM_TIMER_TEST     , 0);

    // Initialize ISR
    attachInterrupt(digitalPinToInterrupt(BM_SPD_IN), encISR_A, RISING);

    // reserve bytes for the inputCmd:
    for(int i=0; i<sub_cmds; i++){
        inputCmd[i].reserve(cmd_buff_size);
    }
    Serial.println("OK");
}

/**
 * Main()
 *
 * @param values Global
 * @return N/A
 */
void bm_run() {

    // Check Serial port for data, parse and run
    if (Serial.available() > 0) {
        getSerial();
        if (inputCmdComplete) {
            Serial.print("Run CMD: ");
            Serial.print(inputCmd[0]);
            for(int i=1; i<sub_cmds; i++){
                if (inputCmd[i] == 0) {
                    break;
                } else {
                    Serial.print(" ");
                    Serial.print(inputCmd[i]);
                }
            }
            Serial.println("");

            cmdRunner();

            // clear the string:
            for(int i=0; i<sub_cmds; i++){
                inputCmd[i] = "";
            }

            inputCmdComplete = false;

        }
    }

    // Check if a full timer sequence has completed, if so calculate RPM
    if (sys_timer_epoch_done == 1) {
        getRPM();
        // digitalWrite(BM_TIMER_TEST, digitalRead(BM_TIMER_TEST) ^ 1);
    }

}





void encISR_A() {
    if (int_timer_epoch_done == 1) {
        int_timer_epoch_done = 0;
        last_rotation_cnt = fract_rotation_cnt;
        fract_rotation_cnt = 0;
    } else {
        fract_rotation_cnt += 1;
    }
}


// 1 second ISR
ISR(TIMER1_OVF_vect) {
    TCNT1 = timer1_counter;   // preload timer
    int_timer_epoch_done = 1;
    sys_timer_epoch_done = 1;
}
/**
 * Calculate RPM
 *
 * @param values N/A
 * @return N/A
 */
void getRPM() {
    if (sys_timer_epoch_done == 1) {
        sys_timer_epoch_done = 0;
        float rev = (float)last_rotation_cnt/30.0;
        // rev /= 0.50055;
        // rev /= (0.5011);
        rev /= (2*0.5011);
        rev *= 60.0;
        RPM = rev;
        printRPM();
    }
}


/**
 * Print RPM to Serial port
 *
 * @param values Global
 * @return N/A
 */
void printRPM() {
    Serial.print((int)RPM);
    Serial.println(" RPM");
}




/**
 * Initialize Timer1
 *
 * @tparam Global variable timer1_counter
 * @return N/A
 */
void timer_1sec_int_init() {
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = timer1_counter;   // preload timer
    TCCR1B |= (1 << CS12);    // 256 prescaler 
    TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
}



/**
 * Executes system functions based on Serial commands
 *
 * @param values Global
 * @return N/A
 */
void cmdRunner() {

    int decodedValue = 0;
    if (inputCmd[0] == "stop_mode") {
        decodedValue = 1;
    }
    if (inputCmd[0] == "fwd") {
        decodedValue = 2;
    }
    if (inputCmd[0] == "rev") {
        decodedValue = 3;
    }
    if (inputCmd[0] == "mode") {
        decodedValue = 4;
    }
    if (inputCmd[0] == "alm_rst") {
        decodedValue = 5;
    }
    if (inputCmd[0] == "run") {
        decodedValue = 6;
    }
    if (inputCmd[0] == "speed") {
        decodedValue = 7;
    }

    // Serial.print("decodedValue: ");
    // Serial.println(decodedValue);

    switch(decodedValue) {

        case 1: //stop
            // digitalWrite(BM_STOP_MODE, BM_INACTIVE);
            Serial.println("stop cmd is no longer supported");
            break;

        case 2: //fwd
            digitalWrite(BM_REV, BM_INACTIVE);
            digitalWrite(BM_FWD, BM_ACTIVE);
            break;

        case 3: //rev
            digitalWrite(BM_FWD, BM_INACTIVE);
            digitalWrite(BM_REV, BM_ACTIVE);
            break;

        case 4: //mode, sub cmds: 0, 1, 2, 3
            Serial.println("mode cmd is no longer supported");
            break;

        case 5: //alm_rst
            digitalWrite(BM_ALM_RST, BM_ACTIVE);
            digitalWrite(BM_FWD, BM_INACTIVE);
            digitalWrite(BM_REV, BM_INACTIVE);
            // Replace delay with while(BM_ALM)
            delay(1000);
            digitalWrite(BM_ALM_RST, BM_INACTIVE);
            break;

        case 6: //run
            // digitalWrite(BM_STOP_MODE, BM_ACTIVE);
            Serial.println("run cmd is no longer supported");
            break;

        case 7: //run
            dutyCycle = inputCmd[1].toInt();
            if (dutyCycle < 0 || dutyCycle > 100) {
                Serial.print("Sub command --> ");
                Serial.print(inputCmd[1]);
                Serial.println(" <-- invalid range! Use 0 - 100");
            } else {
                analogWrite(BM_SPD_OUT, DUTY2PWM(dutyCycle));
                Serial.print("dutyCycle set to: ");
                Serial.println(dutyCycle);
            }
            break;

        default:
            Serial.print("Command --> ");
            Serial.print(inputCmd[0]);
            Serial.println(" <-- invalid!");
            break;

    }
}


/**
 * Collects Serial inputs, parses and formats for later use
 *
 * @tparam inputCmdComplete (write only, global)
 * @tparam inputCmd[] (write only, global)
 * @tparam sub_cmd_idx (write only, global)
 * @return sum of `values`, or 0.0 if `values` is empty.
 */
/*
    Get serial data and remove terminating char '\n'
*/
void getSerial() {
    while (Serial.available()) {
        // get the new byte:
        char inChar = (char)Serial.read();
        // Omit adding '\n' to the string
        if (inChar == '\n') {
            inputCmdComplete = true;
            sub_cmd_idx = 0;
        // Skip spaces and store in next string array
        } else if (inChar == ' ') {
            sub_cmd_idx += 1;
        } else {
            // Populate inputCmd
            inputCmd[sub_cmd_idx] += inChar;
        }
    }
}







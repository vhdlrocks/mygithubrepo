

// Feather 32u4 clock frequency
#define CPU_HZ 8000000 // 8 MHz

// Notes:
// PWM Measured: 526 Hz pulses (1.9 ms)



/*--------------------------------
        IO placement
   -------------------------------*/
//Define Outputs
#define mtr_FWD         18 // was 8 on leo
#define mtr_REV         9
// #define mtr_STOP_MODE   10 // Leave logic pin high (or invert in sw and tie to ground) on controller (prevents instantaneous stops)
// #define mtr_M0          11 // Can remove and leave logic pin low (setting #1, tie low)
// #define mtr_M1          12 // Can remove and use analog input on setting #1
#define mtr_alm_rst     13
#define mtr_analog_spd  11 // PWM output, was 5 on leo
#define timer_test  20 // 1 second interrupt test pin, was 4 on leo

//Define Inputs
#define mtr_speed_in    0 // Was 7 on leo, needs alternate interrupt pin
#define mtr_alm_in      6 // **Not currently being used**

// Output related to brushless motor driver (BLE2D60-A)
#define ACTIVE 0
#define INACTIVE 1

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
 * System setup
 */
void setup() {

    // initialize timer1 
    noInterrupts();           // disable all interrupts
    timer_1sec_int_init();
    interrupts();             // enable all interrupts

    Serial.begin(115200);

    pinMode(mtr_FWD         , OUTPUT);
    pinMode(mtr_REV         , OUTPUT);
    // pinMode(mtr_STOP_MODE      , OUTPUT);
    // pinMode(mtr_M0          , OUTPUT);
    // pinMode(mtr_M1          , OUTPUT);
    pinMode(mtr_alm_rst     , OUTPUT);
    pinMode(mtr_analog_spd  , OUTPUT);
    pinMode(timer_test  , OUTPUT);


    pinMode(mtr_alm_in      , INPUT);
    pinMode(mtr_speed_in    , INPUT);

    digitalWrite(mtr_FWD        , INACTIVE);
    digitalWrite(mtr_REV        , INACTIVE);
    // digitalWrite(mtr_STOP_MODE  , INACTIVE);
    // digitalWrite(mtr_M0         , INACTIVE);
    // digitalWrite(mtr_M1         , INACTIVE);
    digitalWrite(mtr_alm_rst    , INACTIVE);
    digitalWrite(timer_test     , 0);

    // Initialize ISR
    attachInterrupt(digitalPinToInterrupt(mtr_speed_in), encISR_A, RISING);

    // reserve bytes for the inputCmd:
    for(int i=0; i<sub_cmds; i++){
        inputCmd[i].reserve(cmd_buff_size);
    }
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
 * Main()
 *
 * @param values Global
 * @return N/A
 */
void loop() {

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
        // digitalWrite(timer_test, digitalRead(timer_test) ^ 1);
    }

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
            // digitalWrite(mtr_STOP_MODE, INACTIVE);
            Serial.println("stop cmd is no longer supported");
            break;

        case 2: //fwd
            digitalWrite(mtr_REV, INACTIVE);
            digitalWrite(mtr_FWD, ACTIVE);
            break;

        case 3: //rev
            digitalWrite(mtr_FWD, INACTIVE);
            digitalWrite(mtr_REV, ACTIVE);
            break;

        case 4: //mode, sub cmds: 0, 1, 2, 3
            Serial.println("mode cmd is no longer supported");
            break;

        case 5: //alm_rst
            digitalWrite(mtr_alm_rst, ACTIVE);
            digitalWrite(mtr_FWD, INACTIVE);
            digitalWrite(mtr_REV, INACTIVE);
            // Replace delay with while(mtr_ALM_in)
            delay(1000);
            digitalWrite(mtr_alm_rst, INACTIVE);
            break;

        case 6: //run
            // digitalWrite(mtr_STOP_MODE, ACTIVE);
            Serial.println("run cmd is no longer supported");
            break;

        case 7: //run
            dutyCycle = inputCmd[1].toInt();
            if (dutyCycle < 0 || dutyCycle > 100) {
                Serial.print("Sub command --> ");
                Serial.print(inputCmd[1]);
                Serial.println(" <-- invalid range! Use 0 - 100");
            } else {
                analogWrite(mtr_analog_spd, DUTY2PWM(dutyCycle));
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






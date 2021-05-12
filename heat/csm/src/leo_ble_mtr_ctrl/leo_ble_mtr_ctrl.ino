
// Notes:
// PWM Measured: 526 Hz pulses (1.9 ms)
// Leonardo clock frequency: 16 MHz


// Macro to converts from duty (0..100) to PWM (0..255)
#define DUTY2PWM(x)  ((255*(x))/100)

//Define Outputs
#define mtr_FWD         8
#define mtr_REV         9
#define mtr_STOP_MODE   10 // Leave logic pin high (or invert in sw and tie to ground) on controller (prevents instantaneous stops)
#define mtr_M0          11 // Can remove and leave logic pin low (setting #1, tie low)
#define mtr_M1          12 // Can remove and use analog input on setting #1
#define mtr_alm_rst     13
#define mtr_analog_spd  5 // PWM output
#define timer_test  4 // 1 second interrupt test pin


//Define Inputs
#define mtr_speed_in    7
#define mtr_alm_in      6


#define USE_IO_DBG  0

// Serial command variables/constants
#define sub_cmds 10
#define cmd_buff_size 20
String inputCmd[sub_cmds] = "";         // a String to hold incoming data
bool inputCmdComplete = false;  // whether the string is complete
int sub_cmd_idx = 0;

// Active low inputs to remote controller/driver
#define ACTIVE 0
#define INACTIVE 1








// ISR variables
#define ENCODERMULT 30 // Pulses per revolution from the encoder
#define NUM_TIME_SAMPLES 30
volatile unsigned long time_val [NUM_TIME_SAMPLES];
volatile int rpmUpdating = 0;
volatile int encSampleCnt = 0;
// getRPM timing
unsigned long rpmTimeLast = 0;
unsigned long rpmTimeNowMS = 0;
unsigned long last_time_val = 0;
// getRPM related variables
volatile float RPM = 0;

const int RPM_CALC_RATE_MS = 1000; // Report RPM at this interval in milliseconds
#define GEARING 1 // Gear reduction, i.e. 1:20 would be GEARING = 20

int fract_rotation_cnt = 0;
int last_rotation_cnt = 0;
uint8_t int_timer_epoch_done = 0;
uint8_t sys_timer_epoch_done = 0;

void encISR_A() {

    if (int_timer_epoch_done == 1) {
        int_timer_epoch_done = 0;
        last_rotation_cnt = fract_rotation_cnt;
        fract_rotation_cnt = 0;
    } else {
        fract_rotation_cnt += 1;
    }


    // Old
    // if (rpmUpdating == 0){
    //     time_val[encSampleCnt] = micros();
    //     if (encSampleCnt == NUM_TIME_SAMPLES-1) {
    //         encSampleCnt = 0;
    //     } else {
    //         encSampleCnt++;
    //     }
    // }
}

// 1/2-sec timer interrupt
int timer1_counter;

ISR(TIMER1_OVF_vect)        // interrupt service routine 
{
    TCNT1 = timer1_counter;   // preload timer
    int_timer_epoch_done = 1;
    sys_timer_epoch_done = 1;
}



void getRPM() {

    if (sys_timer_epoch_done == 1) {
        sys_timer_epoch_done = 0;
        float rev = (float)last_rotation_cnt/30.0;
        // rev /= 0.50055;
        // rev /= (1.0 - 0.5011);
        rev /= (0.5011);
        rev *= 60.0;
        RPM = rev;
        printRPM();
    }


    // // old
    // // Get time in ms
    // rpmTimeNowMS = millis();
    // if (rpmTimeNowMS - rpmTimeLast > RPM_CALC_RATE_MS) {
    //     if (time_val[0] != 0) { // Skip first value of zero
    //         rpmUpdating = 1; // Let ISR know RPM calculation has started
    //         // unsigned long tempSum = time_val[0] - last_time_val;
    //         unsigned long tempSum = 0;
    //         for(int i = 0; i < NUM_TIME_SAMPLES-1; i++) {
    //             // Todo: check for wrap around values
    //             if (time_val[i+1] > time_val[i]){
    //                 tempSum += (time_val[i+1] - time_val[i]);
    //             }
    //         }
    //         last_time_val = time_val[NUM_TIME_SAMPLES-1];

    //         float rev = tempSum/(NUM_TIME_SAMPLES-1);
    //         rev = 1.0 / rev;            // rev per us
    //         rev *= 1000000;             // rev per sec
    //         rev *= 60;                  // rev per min
    //         rev /= GEARING;             // account for gear ratio
    //         rev /= (ENCODERMULT);         // account for multiple ticks per rotation
    //         RPM = rev;
    //         printRPM();
    //     }
    //     rpmUpdating = 0; // Let ISR know RPM calculation is done
    //     rpmTimeLast = rpmTimeNowMS;
    // }
}


void printRPM() {
    Serial.print((int)RPM);
    Serial.println(" RPM");

}





// Setup system
void setup() {

    // initialize timer1 
    noInterrupts();           // disable all interrupts
    timer_1sec_int();
    interrupts();             // enable all interrupts

    Serial.begin(115200);

    pinMode(mtr_FWD         , OUTPUT);
    pinMode(mtr_REV         , OUTPUT);
    pinMode(mtr_STOP_MODE      , OUTPUT);
    pinMode(mtr_M0          , OUTPUT);
    pinMode(mtr_M1          , OUTPUT);
    pinMode(mtr_alm_rst     , OUTPUT);
    pinMode(mtr_analog_spd  , OUTPUT);
    pinMode(timer_test  , OUTPUT);


    pinMode(mtr_alm_in      , INPUT);
    pinMode(mtr_speed_in    , INPUT);

    digitalWrite(mtr_FWD        , INACTIVE);
    digitalWrite(mtr_REV        , INACTIVE);
    digitalWrite(mtr_STOP_MODE  , INACTIVE);
    digitalWrite(mtr_M0         , INACTIVE);
    digitalWrite(mtr_M1         , INACTIVE);
    digitalWrite(mtr_alm_rst    , INACTIVE);
    digitalWrite(timer_test     , 0);

    // Initialize ISR
    attachInterrupt(digitalPinToInterrupt(mtr_speed_in), encISR_A, RISING);

    // reserve bytes for the inputCmd:
    for(int i=0; i<sub_cmds; i++){
        inputCmd[i].reserve(cmd_buff_size);
    }

}


void timer_1sec_int() {
    TCCR1A = 0;
    TCCR1B = 0;

    // Set timer1_counter to the correct value for our interrupt interval
    //timer1_counter = 64911;   // preload timer 65536-16MHz/256/100Hz
    //timer1_counter = 64286;   // preload timer 65536-16MHz/256/50Hz
    timer1_counter = 34286;   // preload timer 65536-16MHz/256/2Hz
  
    TCNT1 = timer1_counter;   // preload timer
    TCCR1B |= (1 << CS12);    // 256 prescaler 
    TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
}


// Macro to converts from duty (0..100) to PWM (0..255)
#define DUTY2PWM(x)  ((255*(x))/100)

int dutyCycle = 0;
int mod = 0;
// int dutyCycle=127;

void loop() {

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

    if (sys_timer_epoch_done == 1) {
        digitalWrite(timer_test, digitalRead(timer_test) ^ 1);
        // delay(1);
        // digitalWrite(timer_test     , 0);
        // int_timer_epoch_done = 0;
    }

    // if (encSampleCnt == NUM_TIME_SAMPLES-1) getRPM();
    if (sys_timer_epoch_done == 1) getRPM();
}




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

    Serial.print("decodedValue: ");
    Serial.println(decodedValue);

    switch(decodedValue) {

        case 1: //stop
            digitalWrite(mtr_STOP_MODE, INACTIVE);
            break;

        case 2: //fwd
            digitalWrite(mtr_REV, INACTIVE);
            digitalWrite(mtr_FWD, ACTIVE);
            break;

        case 3: //rev
            digitalWrite(mtr_FWD, INACTIVE);
            digitalWrite(mtr_REV, ACTIVE);
            break;

        case 4: //mode0
            if (inputCmd[1] == "0") {
                digitalWrite(mtr_M0, INACTIVE);
                digitalWrite(mtr_M1, INACTIVE);
            } else if (inputCmd[1] == "1") {
                digitalWrite(mtr_M0, ACTIVE);
                digitalWrite(mtr_M1, INACTIVE);
            } else if (inputCmd[1] == "2") {
                digitalWrite(mtr_M0, INACTIVE);
                digitalWrite(mtr_M1, ACTIVE);
            } else if (inputCmd[1] == "3") {
                digitalWrite(mtr_M0, ACTIVE);
                digitalWrite(mtr_M1, ACTIVE);
            } else {
                Serial.print("Sub command --> ");
                Serial.print(inputCmd[1]);
                Serial.println(" <-- invalid!");
            }
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
            digitalWrite(mtr_STOP_MODE, ACTIVE);
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

void DebugRoutine(uint8_t Serialinput) {

    if (USE_IO_DBG) {
        switch(Serialinput) {
            case '1':
                digitalWrite(mtr_FWD, 1);
                delay(2000);
                digitalWrite(mtr_FWD, 0);
                delay(2000);
                digitalWrite(mtr_FWD, 1);
                break;
            case '2':
                digitalWrite(mtr_REV, 1);
                delay(2000);
                digitalWrite(mtr_REV, 0);
                delay(2000);
                digitalWrite(mtr_REV, 1);
                break;
            case '3':
                digitalWrite(mtr_STOP_MODE, 1);
                delay(2000);
                digitalWrite(mtr_STOP_MODE, 0);
                delay(2000);
                digitalWrite(mtr_STOP_MODE, 1);
                break;
            case '4':
                digitalWrite(mtr_M0, 1);
                delay(2000);
                digitalWrite(mtr_M0, 0);
                delay(2000);
                digitalWrite(mtr_M0, 1);
                break;
            case '5':
                digitalWrite(mtr_M1, 1);
                delay(2000);
                digitalWrite(mtr_M1, 0);
                delay(2000);
                digitalWrite(mtr_M1, 1);
                break;
            case '6':
                digitalWrite(mtr_alm_rst, 1);
                delay(2000);
                digitalWrite(mtr_alm_rst, 0);
                delay(2000);
                digitalWrite(mtr_alm_rst, 1);
                break;
            case '7':
                dutyCycle += 10;
                if (dutyCycle > 100) dutyCycle = 0;
                analogWrite(mtr_analog_spd, DUTY2PWM(dutyCycle));
                Serial.print("dutyCycle: ");
                Serial.print(dutyCycle);
                Serial.println("%");
                break;

            default:
                Serial.println("invalid");
        }
    } 
}


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
            // Omit adding '\n' to the string
        } else if (inChar == ' ') {
            sub_cmd_idx += 1;
        } else {
            // add it to the inputCmd:
            inputCmd[sub_cmd_idx] += inChar;
        }

    }
}




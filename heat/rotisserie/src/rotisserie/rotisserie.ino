
// Base libraries
#include <Wire.h>
// Motor libraries
#include <Adafruit_MotorShield.h>
// Display libraries
#include <Adafruit_SSD1306.h>

// Update intervals in milliseconds (ms)
const int RPM_CALC_RATE_MS = 1000; // Report RPM at this interval in milliseconds
const int MTR_UPDATE_RATE_MS = 50; // Update motor speed/direction at this interval

// Pin assignments
const int analogInPin0 = A0; // Analog input pin that the potentiometer is attached to
#define ENCODER_A 7 // Encoder input A (used for speed detection)
#define ENCODER_B 12 // Encoder input B (was going to use for direction, too much phase jitter, unused)
// #define GEARING     20
#define GEARING 1 // Gear reduction, i.e. 1:20 would be GEARING = 20
#define ENCODERMULT 12 // Pulses per revolution from the encoder

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// And connect a DC motor to port M1
Adafruit_DCMotor *myMotor = AFMS.getMotor(1);

// We'll display the speed/direction on the OLED
// Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

// ISR variables
#define NUM_TIME_SAMPLES 12
volatile unsigned long time_val [NUM_TIME_SAMPLES];
volatile int rpmUpdating = 0;
volatile int encSampleCnt = 0;

// getRPM timing
unsigned long rpmTimeLast = 0;
unsigned long rpmTimeNowMS = 0;
// getRPM related variables
volatile float RPM = 0;
volatile bool motordir = FORWARD; // Forward is equal to 1

// setMotorParams timing
unsigned long mtrTimeLast = 0;
unsigned long mtrTimeNowMS = 0;
// setMotorParams related variables
int sensorValue = 0; // Raw value read from the analog input (10-bit ADC)
int motorValue = 0; // Value sent to motor driver IC (PCA9685, 8-bit value)

void encISR_A() {
    // motordir = digitalRead(ENCODER_B);
    if (rpmUpdating == 0){
        time_val[encSampleCnt] = micros();
        if (encSampleCnt == NUM_TIME_SAMPLES-1) {
            encSampleCnt = 0;
        } else {
            encSampleCnt++;
        }
    }
}

void setup() {

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    pinMode(ENCODER_B, INPUT_PULLUP); // Too much jitter, can't use for direction detect
    pinMode(ENCODER_A, INPUT_PULLUP); // Interrupt trigger

    // delay(100);
    // // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    // if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    //     Serial.println(F("SSD1306 allocation failed"));
    //     for(;;); // Don't proceed, loop forever
    // }
    // display.display();
    // display.setTextSize(2);
    // display.setTextColor(WHITE);

    // Initialize motor
    AFMS.begin();  // create with the default frequency 1.6KHz
    // turn on motor M1
    myMotor->setSpeed(0);

    Serial.begin(115200);           // set up Serial library at 115200 bps

    delay(5000);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB
    }
    Serial.println("*******************************");
    Serial.println("Ceramic Rotisserie...");
    Serial.println("*******************************");
    Serial.println("");
    Serial.println("User configurable refresh rates:");
    Serial.println("-------------------------------");
    Serial.print("RPM refresh rate (ms) -> ");
    Serial.println(RPM_CALC_RATE_MS);
    Serial.print("Motor refresh rate (ms) -> ");
    Serial.println(MTR_UPDATE_RATE_MS);
    Serial.println("");
    Serial.println("Pin assignments:");
    Serial.println("-------------------------------");
    Serial.print("Potentiometer -> Pin ");
    String analogPinStr;
    Serial.println("A0");
    Serial.print("ENCODER_A -> Pin ");
    Serial.println(ENCODER_A);
    Serial.print("ENCODER_B -> Pin ");
    Serial.println(ENCODER_B);
    Serial.println("");
    Serial.println("Motor/Encoder configuration:");
    Serial.println("-------------------------------");
    Serial.print("Gear reduction (GEARING): 1/");
    Serial.println(GEARING);
    Serial.print("Encoder PPR (ENCODERMULT): ");
    Serial.println(ENCODERMULT);
    Serial.println("");

    // Initialize ISR
    attachInterrupt(digitalPinToInterrupt(ENCODER_A), encISR_A, RISING);
    delay(100);

    Serial.println("Initialization complete...");
}



void getRPM() {



    // Get time in ms
    rpmTimeNowMS = millis();
    if (rpmTimeNowMS - rpmTimeLast > RPM_CALC_RATE_MS) {
        if (time_val[0] != 0) { // Skip first value of zero
            digitalWrite(!digitalRead(LED_BUILTIN), LOW);
            rpmUpdating = 1; // Let ISR know RPM calculation has started
            unsigned long tempSum = 0;
            for(int i = 0; i < NUM_TIME_SAMPLES-1; i++) {
                // Todo: check for wrap around values
                if (time_val[i+1] > time_val[i]){
                    tempSum += (time_val[i+1] - time_val[i]);
                }
            }

            float rev = tempSum/NUM_TIME_SAMPLES;
            rev = 1.0 / rev;            // rev per us
            rev *= 1000000;             // rev per sec
            rev *= 60;                  // rev per min
            rev /= GEARING;             // account for gear ratio
            rev /= ENCODERMULT;         // account for multiple ticks per rotation
            RPM = rev;

            printRPM();
        }
        rpmUpdating = 0; // Let ISR know RPM calculation is done
        rpmTimeLast = rpmTimeNowMS;
    }
}


void printRPM() {
    // display.clearDisplay();
    // display.setCursor(0,0);
    
    // Serial.print("Direction: ");
    // if (motordir) {
    //     display.println("Forward");
    //     Serial.println("forward @ ");
    // } else {
    //     display.println("Backward");
    //     Serial.println("backward @ ");
    // }
    // display.print((int)RPM); display.println(" RPM");
    // Serial.print((int)RPM); Serial.println(" RPM"); 
    // display.display();

    Serial.print("Direction: ");
    if (motordir) {
      Serial.print("forward @ ");
    } else {
      Serial.print("backward @ ");
    }
    Serial.print((int)RPM);
    Serial.println(" RPM");

}


void setMotorParams() {

    // Get time in ms
    mtrTimeNowMS = millis();

    if (mtrTimeNowMS - mtrTimeLast > MTR_UPDATE_RATE_MS) {

        // read the analog in value:
        sensorValue = analogRead(analogInPin0);

        motorValue = (sensorValue)/2;
        if (motorValue >= 256) {
            myMotor->run(FORWARD);
            motorValue = motorValue - 256;
            myMotor->setSpeed(motorValue);
            motordir = FORWARD;
        } else {
            myMotor->run(BACKWARD);
            motorValue = 255 - motorValue;
            // if (motorValue > 255) motorValue = 255;
            myMotor->setSpeed(motorValue);
            motordir = 0;
        }

        // Serial.print("sensor = ");
        // // Serial.println(sensorValue);
        // Serial.print(sensorValue);
        // Serial.print("\t motorValue = ");
        // Serial.println(motorValue);

        mtrTimeLast = mtrTimeNowMS;
    }
}


void loop() {

    setMotorParams();
    getRPM();

}
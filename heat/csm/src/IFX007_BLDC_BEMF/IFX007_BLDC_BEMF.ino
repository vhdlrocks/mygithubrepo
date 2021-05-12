// Leonardo clock frequency: 16 MHz

// Project: https://github.com/Infineon/IFX007T-Motor-Control/tree/master/examples/IFX007_BLDC_BEMF

/*
Leonardo PWM resource:
http://r6500.blogspot.com/2014/12/fast-pwm-on-arduino-leonardo.html
*/
// Notes:
// 31.28 kHz pulses (32 us)
// default duty cycle: 60%
// Pulse phase length: 1989 us
// Phase frequency: 192 Hz (5.2 ms)

// Macro to converts from duty (0..100) to PWM (0..255)
#define DUTY2PWM(x)  ((255*(x))/100)

// Frequency modes for TIMER1
#define PWM62k   1   //62500 Hz
#define PWM8k    2   // 7812 Hz
#define PWM1k    3   //  976 Hz
#define PWM244   4   //  244 Hz
#define PWM61    5   //   61 Hz

// Change all analogWrite to

// Direct PWM change variables
#define PWM_U OCR1A
#define PWM_V OCR1B
#define PWM_W OCR1C
// Bypass using analogWrite
//Define Inputs & Outputs
// #define PWM_U 11    // PWM output
// #define PWM_V 10    // PWM output
// #define PWM_W 9     // PWM output

#define INH_U 6     // Digital output, inhibit (sleep mode on IFX007T), active low
#define INH_V 5     // Digital output, inhibit (sleep mode on IFX007T), active low
#define INH_W 3     // Digital output, inhibit (sleep mode on IFX007T), active low

// Based on ADC phase voltage inputs:
#define BEMF_U A3
#define BEMF_V A2
#define BEMF_W A1
#define ADC_VS A0


//Define Motor parameters
#define MotorPoles 8    // Not used
#define HallPoleShift 0 // Not used

//StartUp - Commutation-Counts to switch over to closed-loop
#define OpenLoopToClosedLoopCount 50 // Not used
uint8_t BEMF_phase = A1; // Not used
uint8_t ClosedLoop = 0; // Not used


uint8_t Commutation = 0; // Commutation state, 0 - 5 (6 states)
// uint8_t DutyCycle = 150; // 0 - 255, 0% - 100%
uint8_t DutyCycle = 70; // 0 - 255, 0% - 100%
uint8_t Dir = 0;
uint32_t V_neutral = 0;

// Setup system
void setup() {

    Serial.begin(115200);

    // setPwmFrequency(PWM_U, 1);
    // setPwmFrequency(PWM_V, 1);
    // setPwmFrequency(PWM_W, 1);

    // Set ADC sampling time faster in order to be fast enough to detect commutation pulse:
    setADCspeedFast();

    // put your setup code here, to run once:

    pinMode(INH_U, OUTPUT);
    pinMode(INH_V, OUTPUT);
    pinMode(INH_W, OUTPUT);


    pwm91011configure(PWM62k);
    pwmSet9(0);
    pwmSet10(0);
    pwmSet11(0);

    // pinMode(PWM_U, OUTPUT);
    // pinMode(PWM_V, OUTPUT);
    // pinMode(PWM_W, OUTPUT);

    V_neutral = (((uint32_t)analogRead(ADC_VS) * DutyCycle) >> 8);

}

#define INCR 1
#define DECR 0
int cnt_dir = DECR;
uint16_t dly_us = 5000;

#define TEST_RUN 1
void loop() {


    while(TEST_RUN) {
    // uint16_t dly_us = 10000;
    uint8_t CommStartup = 0;

    // Startup procedure: start rotating the field slowly and increase the speed
    while (dly_us>=1000) {

        if (Serial.available() > 0)
        {
            uint8_t in = Serial.read();
            DebugRoutine(in); //To set speed (and Parameters) via Keyboard input.
        }


        delayMicroseconds(dly_us);
        Commutation = CommStartup;
        UpdateHardware (CommStartup,0);
        CommStartup++;
        if (CommStartup==6) CommStartup=0;
        // if (cnt_dir == INCR) dly_us=dly_us+20; else dly_us=dly_us-20;
        // // dly_us=dly_us-20;

        // if ((dly_us < 1000) || (dly_us > 10000)) {
        //     if (cnt_dir == INCR) {
        //         cnt_dir = DECR;
        //      } else {
        //          cnt_dir = INCR;
        //          dly_us = 1000;
        //      }
        // }
        }
    }

    // main loop:
    while(1) {
        if (Serial.available() > 0)
        {
            uint8_t in = Serial.read();
            DebugRoutine(in); //To set speed (and Parameters) via Keyboard input.
        }


        DoCommutation();
    }

}


void DebugRoutine(uint8_t Serialinput)
{
    if (Serialinput == '4')
    {
      DutyCycle+=1;
      Serial.print("DutyCycle: ");
      Serial.println(DutyCycle);
    }
    if (Serialinput == '1')
    {
      DutyCycle-=1;
      Serial.print("DutyCycle: ");
      Serial.println(DutyCycle);
    }


    if (Serialinput == '5')
    {
      dly_us+=100;
      Serial.print("dly_us: ");
      Serial.println(dly_us);
    }
    if (Serialinput == '2')
    {
      dly_us-=100;
      Serial.print("dly_us: ");
      Serial.println(dly_us);
    }

}



void DoCommutation() {
V_neutral = (((uint32_t)analogRead(ADC_VS) * DutyCycle) >> 8);
//V_neutral = analogRead(BEMF_phase);

    if (Dir == 0) {
        switch (Commutation) {
            case 0:
                for (int i=0; i< 20; i++) {
                    if (analogRead(BEMF_W)>V_neutral) i-=1;
                }
                if (analogRead(BEMF_W) < V_neutral) {
                    Commutation = 1;
                    UpdateHardware(Commutation,0);
                }
                break;
            case 1:

                for (int i=0; i< 20; i++) {
                    if (analogRead(BEMF_V)<V_neutral) i-=1;
                }
                if (analogRead(BEMF_V) > V_neutral) {
                    Commutation=2;
                    UpdateHardware(Commutation,0);
                }
                break;
            case 2:

                for (int i=0; i< 20; i++) {
                    if (analogRead(BEMF_U)>V_neutral) i-=1;
                }
                if (analogRead(BEMF_U) < V_neutral) {
                    Commutation=3;
                    UpdateHardware(Commutation,0);
                }
                break;
            case 3:

                for (int i=0; i< 20; i++) {
                    if (analogRead(BEMF_W)<V_neutral) i-=1;
                }
                if (analogRead(BEMF_W) > V_neutral) {
                    Commutation=4;
                    UpdateHardware(Commutation,0);
                }
                break;
            case 4:

                for (int i=0; i< 20; i++) {
                    if (analogRead(BEMF_V)>V_neutral) i-=1;
                }
                if (analogRead(BEMF_V) < V_neutral) {
                    Commutation=5;
                    UpdateHardware(Commutation,0);
                }
                break;
            case 5:

                for (int i=0; i< 20; i++) {
                    if (analogRead(BEMF_U)<V_neutral) i-=1;
                }
                if (analogRead(BEMF_U) > V_neutral) {
                    Commutation=0;
                    UpdateHardware(Commutation,0);
                }
                break;
            defailt:
            break;

        }
    }
}


//defining commutation steps according to HALL table
void UpdateHardware(uint8_t CommutationStep, uint8_t Dir) {

    // update neutral voltage:
    V_neutral = (int)(((uint32_t)analogRead(ADC_VS) * DutyCycle) >> 8);


    //CW direction
    if (Dir == 0) {

        switch (CommutationStep) {
            case 0:
                digitalWrite(INH_U, HIGH);
                digitalWrite(INH_V, HIGH);
                digitalWrite(INH_W, LOW);
                PWM_U=DutyCycle;
                PWM_V=0;
                PWM_W=0;
                // analogWrite(PWM_U, DutyCycle);
                // analogWrite(PWM_V, 0);
                // analogWrite(PWM_W, 0);
                break;

            case 1:
                digitalWrite(INH_U, HIGH);
                digitalWrite(INH_V, LOW);
                digitalWrite(INH_W, HIGH);
                PWM_U=DutyCycle;
                PWM_V=0;
                PWM_W=0;
                // analogWrite(PWM_U, DutyCycle);
                // analogWrite(PWM_V, 0);
                // analogWrite(PWM_W, 0);
                break;

            case 2:
                digitalWrite(INH_U, LOW);
                digitalWrite(INH_V, HIGH);
                digitalWrite(INH_W, HIGH);
                PWM_U=0;
                PWM_V=DutyCycle;
                PWM_W=0;
                // analogWrite(PWM_U, 0);
                // analogWrite(PWM_V, DutyCycle);
                // analogWrite(PWM_W, 0);
                break;

            case 3:
                digitalWrite(INH_U, HIGH);
                digitalWrite(INH_V, HIGH);
                digitalWrite(INH_W, LOW);
                PWM_U=0;
                PWM_V=DutyCycle;
                PWM_W=0;
                // analogWrite(PWM_U, 0);
                // analogWrite(PWM_V, DutyCycle);
                // analogWrite(PWM_W, 0);
                break;

            case 4:
                digitalWrite(INH_U, HIGH);
                digitalWrite(INH_V, LOW);
                digitalWrite(INH_W, HIGH);
                PWM_U=0;
                PWM_V=0;
                PWM_W=DutyCycle;
                // analogWrite(PWM_U, 0);
                // analogWrite(PWM_V, 0);
                // analogWrite(PWM_W, DutyCycle);
                break;

            case 5:
                digitalWrite(INH_U, LOW);
                digitalWrite(INH_V, HIGH);
                digitalWrite(INH_W, HIGH);
                PWM_U=0;
                PWM_V=0;
                PWM_W=DutyCycle;
                // analogWrite(PWM_U, 0);
                // analogWrite(PWM_V, 0);
                // analogWrite(PWM_W, DutyCycle);
                break;

            default:
                break;
        }

    }
    else {
        //CCW direction
        switch (CommutationStep) {
            case 0:
                digitalWrite(INH_U, LOW);
                digitalWrite(INH_V, HIGH);
                digitalWrite(INH_W, HIGH);
                PWM_U=0;
                PWM_V=DutyCycle;
                PWM_W=0;
                // analogWrite(PWM_U, 0);
                // analogWrite(PWM_V, DutyCycle);
                // analogWrite(PWM_W, 0);
                break;

            case 1:
                digitalWrite(INH_U, HIGH);
                digitalWrite(INH_V, LOW);
                digitalWrite(INH_W, HIGH);
                PWM_U=DutyCycle;
                PWM_V=0;
                PWM_W=0;
                // analogWrite(PWM_U, DutyCycle);
                // analogWrite(PWM_V, 0);
                // analogWrite(PWM_W, 0);
                break;

            case 2:
                digitalWrite(INH_U, HIGH);
                digitalWrite(INH_V, HIGH);
                digitalWrite(INH_W, LOW);
                PWM_U=DutyCycle;
                PWM_V=0;
                PWM_W=0;
                // analogWrite(PWM_U, DutyCycle);
                // analogWrite(PWM_V, 0);
                // analogWrite(PWM_W, 0);
                break;

            case 3:
                digitalWrite(INH_U, LOW);
                digitalWrite(INH_V, HIGH);
                digitalWrite(INH_W, HIGH);
                PWM_U=0;
                PWM_V=0;
                PWM_W=DutyCycle;
                // analogWrite(PWM_U, 0);
                // analogWrite(PWM_V, 0);
                // analogWrite(PWM_W, DutyCycle);
                break;

            case 4:
                digitalWrite(INH_U, HIGH);
                digitalWrite(INH_V, LOW);
                digitalWrite(INH_W, HIGH);
                PWM_U=0;
                PWM_V=0;
                PWM_W=DutyCycle;
                // analogWrite(PWM_U, 0);
                // analogWrite(PWM_V, 0);
                // analogWrite(PWM_W, DutyCycle);
                break;

            case 5:
                digitalWrite(INH_U, HIGH);
                digitalWrite(INH_V, HIGH);
                digitalWrite(INH_W, LOW);
                PWM_U=0;
                PWM_V=DutyCycle;
                PWM_W=0;
                // analogWrite(PWM_U, 0);
                // analogWrite(PWM_V, DutyCycle);
                // analogWrite(PWM_W, 0);
                break;

            default:
                break;
        }
    }
}

/**
   Divides a given PWM pin frequency by a divisor.

   The resulting frequency is equal to the base frequency divided by
   the given divisor:
     - Base frequencies:
        o The base frequency for pins 3, 9, 10, and 11 is 31250 Hz.
        o The base frequency for pins 5 and 6 is 62500 Hz.
     - Divisors:
        o The divisors available on pins 5, 6, 9 and 10 are: 1, 8, 64,
          256, and 1024.
        o The divisors available on pins 3 and 11 are: 1, 8, 32, 64,
          128, 256, and 1024.

   PWM frequencies are tied together in pairs of pins. If one in a
   pair is changed, the other is also changed to match:
     - Pins 5 and 6 are paired on timer0
     - Pins 9 and 10 are paired on timer1
     - Pins 3 and 11 are paired on timer2

   Note that this function will have side effects on anything else
   that uses timers:
     - Changes on pins 3, 5, 6, or 11 may cause the delay() and
       millis() functions to stop working. Other timing-related
       functions may also be affected.
     - Changes on pins 9 or 10 will cause the Servo library to function
       incorrectly.

   Thanks to macegr of the Arduino forums for his documentation of the
   PWM frequency divisors. His post can be viewed at:
     https://forum.arduino.cc/index.php?topic=16612#msg121031
*/
void setPwmFrequency(int pin, int divisor) {
    byte mode;
    // if (pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    if (pin == 5 || pin == 6 || pin == 9 || pin == 10 || pin == 11) {
        switch (divisor) {
            case 1: mode = 0x01; break;
            case 8: mode = 0x02; break;
            case 64: mode = 0x03; break;
            case 256: mode = 0x04; break;
            case 1024: mode = 0x05; break;
            default: return;
        }
        if (pin == 5 || pin == 6) {
            TCCR0B = TCCR0B & 0b11111000 | mode;
        } else {
            TCCR1B = TCCR1B & 0b11111000 | mode;
        }
    } else if (pin == 3 || pin == 11) {
        switch (divisor) {
            case 1: mode = 0x01; break;
            case 8: mode = 0x02; break;
            case 32: mode = 0x03; break;
            case 64: mode = 0x04; break;
            case 128: mode = 0x05; break;
            case 256: mode = 0x06; break;
            case 1024: mode = 0x07; break;
            default: return;
        }
        // TCCR2B = TCCR2B & 0b11111000 | mode; // Arduino UNO
    }
}


void setADCspeedFast(){

    //Source: https://forum.arduino.cc/index.php?topic=6549.0

    // defines for setting and clearing register bits
    #ifndef cbi
    #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
    #endif
    #ifndef sbi
    #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
    #endif

    // set prescale to 16
    sbi(ADCSRA,ADPS2) ;
    cbi(ADCSRA,ADPS1) ;
    cbi(ADCSRA,ADPS0) ;
}



// Configure the PWM clock
// The argument is one of the 5 previously defined modes
void pwm91011configure(int mode)
{
    // TCCR1A configuration
    //  00 : Channel A disabled D9
    //  00 : Channel B disabled D10
    //  00 : Channel C disabled D11
    //  01 : Fast PWM 8 bit
    TCCR1A=1;
    // TCCR1A|=0x40;  // Fast mode, jeff

    // TCCR1B configuration
    // Clock mode and Fast PWM 8 bit
    TCCR1B=mode|0x08;

    // TCCR1C configuration
    TCCR1C=0;
}

// Set PWM to D9
// Argument is PWM between 0 and 255
void pwmSet9(int value)
{
    OCR1A=value;   // Set PWM value
    DDRB|=1<<5;    // Set Output Mode B5
    TCCR1A|=0x80;  // Activate channel
}

// Set PWM to D10
// Argument is PWM between 0 and 255
void pwmSet10(int value)
{
    OCR1B=value;   // Set PWM value
    DDRB|=1<<6;    // Set Output Mode B6
    TCCR1A|=0x20;  // Set PWM value
}

// Set PWM to D11
// Argument is PWM between 0 and 255
void pwmSet11(int value)
{
    OCR1C=value;   // Set PWM value
    DDRB|=1<<7;    // Set Output Mode B7
    TCCR1A|=0x08;  // Set PWM value
}

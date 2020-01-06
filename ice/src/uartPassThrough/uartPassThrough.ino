
// Signal list
int RTS         = 11;
int DTR         = 10;   // Modem change, WAKE signal
int RESET_N     = 9;
int IO8         = 8;
int L           = 13;
int RLY1_CTRL   = 5;
int D6          = 6;
int ON_OFF      = 12;
int D4          = 4;
int D2          = 2;
int D3          = 3;



void setup() {

    pinMode(RESET_N, INPUT);

    pinMode(DTR, OUTPUT);
    digitalWrite(DTR, LOW);

    pinMode(RTS, OUTPUT);
    digitalWrite(DTR, LOW);

    pinMode(ON_OFF, INPUT);


    // initialize serial debug communication with PC over USB connection
    Serial.begin(9600);
    while (!Serial) ; // wait for serial debug port to connect to PC
    Serial.println("Serial Port Initialized");

    // open hardware UART
    // Serial1.begin(115200);
    // Serial1.begin(921600);
    Serial1.begin(921600 - 50000);
    while (!Serial1) ; // wait for serial debug port to connect to PC

    Serial.println("Initializing Modem");

    // cellModemOff();
    // cellModemOn();

    Serial.println("Ready for UART passthrough");
}

void loop() {

    while (Serial1.available() > 0)
    {
        //read incoming byte from modem and write byte out to debug serial over USB
        Serial.write(Serial1.read());
    }

    while (Serial.available() > 0)
    {
        //read incoming byte from modem and write byte out to debug serial over USB
        Serial1.write(Serial.read());
    }

}



/*******************************************************************************************
*
* cellModemOn
*
*******************************************************************************************/
void cellModemOn(void)
{
    // Start cellular modem
    Serial.println("Starting Cellular Modem");
    // Arduino I/O pin 12 is connected to modem ON_OFF signal.
    // ON_OFF has internal 200k pullup resister and needs to be driven low by external signal for >1s to startup
    // Arduino defaults to I/O pin as input with no pullup on reset
    // Set I/O pin 12 to low
    digitalWrite(ON_OFF, LOW);

    // Configure I/O pin 12 as output
    pinMode(ON_OFF, OUTPUT);

    // Drive I/O pin 12 low
    digitalWrite(ON_OFF, LOW);
    delay(1100); // modem requires >1s pulse

    // Return I/O pin 12 to input/hi-Z state
    pinMode(ON_OFF, INPUT);

    // Requires 10 seconds after reset
    //delay(10000);
    delay(1000);
}

/*******************************************************************************************
*
* cellModemOff
*
*******************************************************************************************/
void cellModemOff(void)
{
    Serial.println("Resetting Cellular Modem");
    digitalWrite(ON_OFF, LOW);

    // Configure I/O pin 12 as output
    pinMode(ON_OFF, OUTPUT);

    // Drive I/O pin 12 low
    digitalWrite(ON_OFF, LOW);
    delay(2100); // modem requires >1s pulse

    // Return I/O pin 12 to input/hi-Z state
    pinMode(ON_OFF, INPUT);

    // Requires 10 seconds after reset
    //delay(10000);
    delay(1000);
}

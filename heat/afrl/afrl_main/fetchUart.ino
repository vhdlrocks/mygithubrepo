


/*--------------------------------
        getSerial() global variables/constants
   -------------------------------*/
const uint8_t sub_cmds = 30;
const uint8_t cmd_buff_size = 20;
String inputCmd[sub_cmds] = ""; // a String to hold incoming data
bool inputCmdComplete = false;  // whether the string is complete
int sub_cmd_idx = 0;

const uint8_t NUM_ELEMENTS = 4;
const uint8_t SIZE_ELEMENTS = 10;
char cmdList[NUM_ELEMENTS][SIZE_ELEMENTS] = {
    "vars",
    "run",
    "test",
    "help"
};

const char * cmdListHelp[] = {
// const char cmdListHelp[] = {
    "System variables.",
    "Run configuration.",
    "Test configuration.",
    "Help menu."
};

const uint8_t NUM_ELEMENTS_vars = 3;
const uint8_t SIZE_ELEMENTS_vars = 10;
// char * cmdListSubHelp[] = {
const char cmdListSub_vars[NUM_ELEMENTS_vars][SIZE_ELEMENTS_vars] = {
    "set",
    "show",
    "help"
};


const char * cmdListSubHelp_vars[] = {
// const char cmdListSubHelp_vars[] = {
    "Sets variable number.",
    "Lists current variable settings.",
    "Help menu and examples."
};


/**
 * fetchUart()
 *
 * @param values Global
 * @return N/A
 */
void fetchUart() {

    // Counts the number of space delimited commands/arguments
    // Max numCmds is set by sub_cmds
    uint8_t numCmds = 0;

    // Check Serial port for data, parse and run
    if (Serial.available() > 0) {
        getSerial();
        if (inputCmdComplete) {
            Serial.print(F(">> "));
            Serial.print(inputCmd[0]);
            for(uint8_t i=1; i<sub_cmds; i++){
                if (inputCmd[i] == 0) {
                    numCmds = i;
                    break;
                } else {
                    Serial.print(" <opt> ");
                    Serial.print(i);
                    Serial.print(": ");
                    Serial.print(inputCmd[i]);
                }
            }
            Serial.println();

            cmdRunner(numCmds);

            // clear the string:
            for(int i=0; i<sub_cmds; i++){
                inputCmd[i] = "";
            }

            inputCmdComplete = false;

        }
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



// 725-1819-ND Solenoid
// 1528-2126-ND M0 board
/**
 * Executes system functions based on Serial commands
 *
 * @param values Global
 * @return N/A
 */
void cmdRunner(uint8_t numCmds) {

    uint8_t decodedValue = 10;
    uint8_t i, j, k = 0;

    for(i=0;i<NUM_ELEMENTS;i++) {
        if (inputCmd[0] == cmdList[i]) {
            decodedValue = i;
        }
    }

    switch(decodedValue) {

        case 0: //vars
            if (inputCmd[1] == "") {
                Serial.println();
                Serial.println(F("Usage:"));
                Serial.print(F("    ")); Serial.print(cmdList[decodedValue]); Serial.println(F(" [var_num] [var_val] .. [var_num N] [var_val N]"));
                Serial.println();
                Serial.println(F("Arguments:"));

                // Get max string leng for formatting help output
                uint8_t linSize[NUM_ELEMENTS_vars];
                for(i=0;i<NUM_ELEMENTS_vars;i++) {
                    for(k=0;k<10;k++) {
                        linSize[i] = k;
                        if (cmdListSub_vars[i][k] == 0) break;
                    }
                }

                // Format help output
                for(i=0;i<NUM_ELEMENTS_vars;i++) {
                    Serial.print(F("    "));
                    Serial.print(cmdListSub_vars[i]);
                    for(j=0;j<SIZE_ELEMENTS_vars-linSize[i];j++) Serial.print(F(" "));
                    Serial.println(cmdListSubHelp_vars[i]);
                }

            } else if (inputCmd[1] == "set") {
                // Check for valid syntax and that each var_num has a valid var_val
                Serial.println(F("numCmds: ")); Serial.println(numCmds);
                bool varsError = false;
                uint8_t numCmdsLocal = numCmds - 2; // Subtract vars set (2 cmds)
                if (numCmdsLocal % 2) {
                    Serial.println(F("mod: ")); Serial.println(numCmdsLocal % 2);
                    varsError = true;
                } else {
                    for(i=2;i<numCmds;i++){
                        if (inputCmd[i].toInt() == 0) {
                            // Add check to make sure the user sent "0" or if it's not an int
                            if (inputCmd[i] != "0") {
                                Serial.print(F("val: ")); Serial.println(inputCmd[i]);
                                Serial.print(F("toInt: ")); Serial.print(inputCmd[i].toInt());
                                varsError = true;
                                break;
                            }
                        }
                    }
                }

                if (varsError) {
                    Serial.println(F("Invalid syntax and/or arguments: arguments must be"));
                    Serial.println(F(" integers and each var_num requires a var_val"));
                    break;
                }

                int var_num, var_val;
                for(i=2;i<numCmds;i+=2){
                    var_num = inputCmd[i].toInt();
                    var_val = inputCmd[i+1].toInt();
                    switch(var_num) {
                        // <SYSTEM VARIABLES>
                        case 0:
                            CAM_A_ON = var_val;
                            break;
                        case 1:
                            CAM_B_ON = var_val;
                            break;
                        case 2:
                            RLY_0_ON = var_val;
                            break;
                        case 3:
                            RLY_0_OFF = var_val;
                            break;
                        case 4:
                            CAM_A_OFF = var_val;
                            break;
                        case 5:
                            CAM_B_OFF = var_val;
                            break;
                        case 6:
                            LED_RATE_SLOW = var_val;
                            break;
                        case 7:
                            LED_RATE_FAST = var_val;
                            break;

                    }
                }

            } else if (inputCmd[1] == "show") {
                Serial.println();
                Serial.println(F("# Variable        : Value"));
                Serial.println(F("-------------------------"));
                // <SYSTEM VARIABLES>
                Serial.print  (F("0 CAM_A_ON        : ")); Serial.println(CAM_A_ON);
                Serial.print  (F("1 CAM_B_ON        : ")); Serial.println(CAM_B_ON);
                Serial.print  (F("2 RLY_0_ON        : ")); Serial.println(RLY_0_ON);
                Serial.print  (F("3 RLY_0_OFF       : ")); Serial.println(RLY_0_OFF);
                Serial.print  (F("4 CAM_A_OFF       : ")); Serial.println(CAM_A_OFF);
                Serial.print  (F("5 CAM_B_OFF       : ")); Serial.println(CAM_B_OFF);
                Serial.print  (F("6 LED_RATE_SLOW   : ")); Serial.println(LED_RATE_SLOW);
                Serial.print  (F("7 LED_RATE_FAST   : ")); Serial.println(LED_RATE_FAST);
                Serial.println(F("-------------------------"));

            } else if (inputCmd[1] == "help") {
                Serial.println();
                Serial.println(F("---------------------------------------------------------------------------------------"));
                Serial.println(F("# Variable        : Description"));
                Serial.println(F("---------------------------------------------------------------------------------------"));
                // <SYSTEM VARIABLES>
                Serial.println(F("0 CAM_A_ON        : Time sequence to turn camera ON, set to 0 to disable"));
                Serial.println(F("1 CAM_B_ON        : Time sequence to turn camera ON, set to 0 to disable"));
                Serial.println(F("2 RLY_0_ON        : Time sequence to turn relay ON, set to 0 to disable"));
                Serial.println(F("3 RLY_0_OFF       : Time sequence to turn relay OFF"));
                Serial.println(F("4 CAM_A_OFF       : Time sequence to turn camera OFF"));
                Serial.println(F("5 CAM_B_OFF       : Time sequence to turn camera OFF"));
                Serial.println(F("6 LED_RATE_SLOW   : LED Blink rate after relay is turned off"));
                Serial.println(F("7 LED_RATE_FAST   : LED Blink rate before relay is turned on"));
                Serial.println(F("---------------------------------------------------------------------------------------"));
                Serial.println(F("All variables are in milliseconds."));
                Serial.println(F("Example of how to change a variable and then run the program: "));
                Serial.println(F("    >> vars set 3 1200 "));
                Serial.println(F("    >> run"));
                Serial.println(F("    This changes the variable RLY_0_ON to actuate on a new delay of 1.2 ms."));
                Serial.println(F("---------------------------------------------------------------------------------------"));
            } else {
                Serial.println(F("Invalid syntax and/or argument. See >> vars help"));
            }
            break;

        case 1: //run
            if (inputCmd[1] == "") {
                Serial.println();
                Serial.println(F("Usage:"));
                Serial.print(F("    ")); Serial.print(cmdList[decodedValue]); Serial.println(F(" [test_num]"));
                Serial.println();
                Serial.println(F("Arguments:"));
                Serial.println(F("    1    Test number to run, ex. >> run 1"));
                Serial.println();
                Serial.println(F("Info:"));
                Serial.println(F("    Running >> run [test_num] will start a dialog and wait to run the"));
                Serial.println(F("    test until ENTER/Send is input from the console. Typing any other"));
                Serial.println(F("    character will cancel the test."));
            // <TEST> Copy/Paste for additional tests
            } else if (inputCmd[1] == "1") {
                userTestRunWait(1);
            // <ADD TEST> This is an example of what test 2 would look like. The actual test structure
            //     is in afrl_main.
            // } else if (inputCmd[1] == "2") {
            //     userTestRunWait(2);
            } else {
                Serial.print(F("Test ")); Serial.print(inputCmd[1]); Serial.println(F(" not loaded into memory."));
            }

            break;

        case 2: //test
            Serial.println("Add test code here.");

            //<EEPROM>
            // if (inputCmd[1] == "write") {
            //     eepWrite();
            // }

            // if (inputCmd[1] == "read") {
            //     eepRead(true);
            // }
            // break;

        case 3: //help
            Serial.println();
            Serial.println(F("Usage:"));
            Serial.println(F("    <command> [option 1] [option 2] .. [option N]"));
            Serial.println();
            Serial.println(F("Commands:"));

            // Get max string leng for formatting help output
            uint8_t linSize[NUM_ELEMENTS];
            for(i=0;i<NUM_ELEMENTS;i++) {
                for(k=0;k<10;k++) {
                    linSize[i] = k;
                    if (cmdList[i][k] == 0) break;
                }
            }

            // Format help output
            for(i=0;i<NUM_ELEMENTS;i++) {
                Serial.print(F("    "));
                Serial.print(cmdList[i]);
                for(j=0;j<SIZE_ELEMENTS-linSize[i];j++) Serial.print(F(" "));
                Serial.println(cmdListHelp[i]);
            }
            Serial.println();
            Serial.println(F("Type the <command> with no options for additional info."));
            break;

        default:
            Serial.print(F("Command --> "));
            Serial.print(inputCmd[0]);
            Serial.println(F(" <-- invalid!"));
            break;

    }
}


void  userTestRunWait(uint8_t testNum) {

    Serial.println(F("Waiting on input: "));
    while (!Serial.available());
    // get the new byte:
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
        RUN_TEST_NUM = testNum;
        RUN_ACTIVE = true;
    } else {
        Serial.println(F("run cancelled by user..exiting run."));
        while (Serial.available()) (char)Serial.read();
    }

}
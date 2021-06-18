








void init_sd_card() {
    while (!Serial);
    Serial.print(F("Initializing SD Card..."));
    if (!SD.begin(4)) {
        
        Serial.println("SD Card init failed!");
        while (1);
    }
    Serial.print(F("OK"));

    root = SD.open("/");

    printDirectory(root, 0);

    Serial.println(F("SD Card done!"));
}

void printDirectory(File dir, int numTabs) {
    while (true) {

        File entry =  dir.openNextFile();
        if (! entry) {
            // no more files
            break;
        }
        for (uint8_t i = 0; i < numTabs; i++) {
            Serial.print('\t');
        }
        Serial.print(entry.name());
        if (entry.isDirectory()) {
            Serial.println("/");
            printDirectory(entry, numTabs + 1);
        } else {
            // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}
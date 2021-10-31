void setup() {
    Serial.begin(9600);
}

/*void loop() {
    if(Serial.available() == 11)
        Serial.print(Serial.readString());
    if(Serial.available() == 12) {
        const char expected[12] = "HELLO WORLD";
        char toWrite[12];
        if(Serial.availableForWrite() == 64) {
            for (int i = 0; i < sizeof(expected); i++) {
                if (Serial.peek() == expected[i]) {
                    toWrite[i] = Serial.read();
                }
            }
            for (int i = 0; i < sizeof(toWrite); i++){
                Serial.print(toWrite[i]);
            }
        }
    }
}*/

void loop() {
    if(Serial.available() == 11) {
        char expected[] = "HELLO";
        char toWrite[5];

        if(Serial.availableForWrite() == 64 && Serial.peek() == expected[0]) {
            for (int i = 0; i < sizeof(expected); i++) {
                if (Serial.peek() == expected[i]) {
                    toWrite[i] = Serial.read();
                }
            }
            String rest = Serial.readString();
            for (int i = 0; i < sizeof(toWrite); i++){
                Serial.print(toWrite[i]);
            }
            Serial.print(rest);
        }
        else if(Serial.peek() == '\0'){
            Serial.print(Serial.readString());
        }
    }
}

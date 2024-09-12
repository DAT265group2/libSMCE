void setup() {
    Serial.begin(9600);
}

void loop() {
    char expected[] = "HELLO UART";
    if(Serial.available() == 11 && Serial.peek() == expected[0]) {
        char toWrite[11];

        for (int i = 0; i < sizeof(expected); i++) {
            if (Serial.peek() == expected[i]) {
                toWrite[i] = Serial.read();
            }
        }
        for (int i = 0; i < sizeof(toWrite); i++){
            Serial.print(toWrite[i]);
        }
    }
    else
        Serial.print(Serial.readString());
}

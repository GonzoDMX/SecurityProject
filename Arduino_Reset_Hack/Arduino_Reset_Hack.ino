
#define PIN 7
#define LED 13

void setup() {
    Serial.begin(115200);
    pinMode(PIN, OUTPUT);
    pinMode(LED, OUTPUT);
    digitalWrite(PIN, HIGH);
    digitalWrite(LED, LOW);
}

char r_check[5] = {'R', 'E', 'S', 'E', 'T'};
char r[5];

bool checkR() {
    for (int i = 0; i < 5; ++i) {
        if(r[i] != r_check[i]) {
            return false;
        }
    }
    return true;
}

void loop() {
    while (Serial.available() > 4) {
        for (int i = 0; i < 5; ++i) {
            r[i] = Serial.read();
        }
        while(Serial.available()) {
            Serial.read();
        }
        if (checkR()) {
            Serial.print("Resetting... ");
            digitalWrite(PIN, LOW);
            digitalWrite(LED, HIGH);
            delay(1000);
            Serial.println("Done.");
            digitalWrite(PIN, HIGH);
            digitalWrite(LED, LOW);
        }
    }
}

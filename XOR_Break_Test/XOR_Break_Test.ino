
/*
 *  Way to break encryption when IV is reused:
 *  Step 1: XOR the first encrypted string with its plaintext
 *  Step 2: XOR the result with the second encrypted string
 * 
 *  PlainText2 = CipherText2 ^ (PlainText1 ^ CipherText1)
 * 
 *  Tips:
 *  IV reuse can be detected when similar patterns emerge in encrypted strings
 *  ex. If the same IV is used,
 *      the same encrypted word will be represented with the same bytes across multiple messages
 *  
 */



byte test1[16] = { 0xA5, 0x2D, 0xA7, 0x88, 0xEF, 0x9F, 0x6B, 0xDC,
                   0x66, 0x60, 0xEC, 0x69, 0xF7, 0x9A, 0xB4, 0x02 };
byte test2[16] = { 0xA6, 0x2D, 0xA7, 0x88, 0xEF, 0x9F, 0x6B, 0xDC,
                   0x66, 0x60, 0xEC, 0x69, 0xF7, 0x81, 0xAD, 0x08 };

byte testP[16] = { 0x02, 0x49, 0x20, 0x61, 0x6D, 0x20, 0x61, 0x20,
                   0x74, 0x65, 0x73, 0x74, 0x20, 0x6F, 0x6E, 0x65};

byte testIV[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

byte container[16] = { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x19, 0x0A };


byte doubT[16] = { 0xA7, 0x64, 0x87, 0xE9, 0x82, 0xBF, 0x0A, 0xFC,
                   0x12, 0x05, 0x9F, 0x1D, 0xD7, 0xF5, 0xDA, 0x67 };

String testPT = " I am a test one";

byte a, b;




void setup() {
  Serial.begin(115200);
/*
  Serial.println("Test 1 XOR Test2");
  for (int i = 0; i < 16; i++) {
    Serial.print(" ");
    a = test1[i] ^ test2[i];
    Serial.print(a, HEX);
    if ((i + 1) % 8 == 0) {
      Serial.println();
    }
  }
  Serial.println("/--------------------/");
  Serial.println();
  delay(50);
  */
  Serial.println("XOR Test IV");
  for (int j = 0; j < 16; j++) {
    Serial.print(" ");
    b = doubT[j] ^ test2[j];
    Serial.print(b, HEX);
    if ((j + 1) % 8 == 0) {
      Serial.println();
    }
  }
  Serial.println("/--------------------/");

}

void loop() {
  // put your main code here, to run repeatedly:

}

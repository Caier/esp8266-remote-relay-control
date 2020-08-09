#include <Arduino.h>

extern const uint8_t ledY, ledG;

void err(String msg, uint8_t Ycnt, uint8_t Gcnt) {
    Serial.println("\nERROR: " + msg);
    while(1) {
        for(int i = 0; i < Ycnt * 2; i++) {
            digitalWrite(ledY, i % 2 != 0);
            delay(500);
        }
        digitalWrite(ledY, 0);
        for(int i = 0; i < Gcnt * 2; i++) {
            digitalWrite(ledG, i % 2 != 0);
            delay(500);
        }
        digitalWrite(ledG, 0);
    }
}
#include <ESP32Servo.h>

// Pin Definitions
const int PB_PIN = 12;   // Pushbutton
const int IR_SRV1 = 10;  // Forward IR
const int IR_SRV2 = 11;  // Reverse IR
const int SRV1_PIN = 6;  // Servo 1
const int SRV2_PIN = 7;  // Servo 2

// State Variables
bool shuffleEnabled = false;

Servo myServo1;
Servo myServo2;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("--- PCB System: Waiting for Button Press ---");

  pinMode(PB_PIN, INPUT);
  pinMode(IR_SRV1, INPUT);
  pinMode(IR_SRV2, INPUT);

  myServo1.attach(SRV1_PIN);
  myServo2.attach(SRV2_PIN);
  
  myServo1.writeMicroseconds(1500);
  myServo2.writeMicroseconds(1500);
}

void loop() {
  if (digitalRead(PB_PIN) == LOW) {
    if (!shuffleEnabled) {
      shuffleEnabled = true;
      Serial.println("GO: Shuffle Mode Activated");
      delay(500); // Debounce delay
    }
  }

  if (shuffleEnabled) {
    int state1 = digitalRead(IR_SRV1);
    int state2 = digitalRead(IR_SRV2);

    if (state1 == LOW || state2 == LOW) {
      // Card detected Spin motors
      myServo1.writeMicroseconds(1700); 
      myServo2.writeMicroseconds(1300); 
    } 
    else {
      // No card Stop motors
      myServo1.writeMicroseconds(1500);
      myServo2.writeMicroseconds(1500);
    }

    //slow down serial spam
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 500) {
      Serial.println("System Active: Monitoring IR sensors...");
      lastDebug = millis();
    }
  }
}

#include <ESP32Servo.h>

//pins const
const int SENSOR_1_PIN = 23;
const int SERVO_1_PIN  = 19;
const int SENSOR_2_PIN = 22;
const int SERVO_2_PIN  = 18;

// pins reassigned
#define ENCODER_A   25
#define ENCODER_B   26
#define EN_PIN      27
#define STEP_PIN    32
#define DIR_PIN     33

// Tune STEPS_PER_MM after  calibration
#define STEPS_PER_MM     25.0
#define DECK_HEIGHT_MM   94.0 
#define STEP_DELAY_US    500    // microseconds between steps (speed)

// ENCODER STATE
volatile int encoderCount = 0;
int lastEncoderCount      = -1;

Servo servo1;
Servo servo2;

int servo1Stop = 1500;
int servo2Stop = 1580;

int count1 = 0;
int count2 = 0;
bool lastState1 = HIGH;
bool lastState2 = HIGH;

// ENCODER INTERRUPT
void IRAM_ATTR encoderISR() {
  if (digitalRead(ENCODER_B) == HIGH)
    encoderCount++;
  else
    encoderCount--;
  // Clamp to 0-24 (full range)
  encoderCount = constrain(encoderCount, 0, 24);
}


void stepMotor(int steps, bool direction) {
  digitalWrite(DIR_PIN, direction);
  delayMicroseconds(2); // direction settle time
  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(STEP_DELAY_US);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(STEP_DELAY_US);
  }
}

// Map encoder count (0-24) to penetration percent (10-90%)
float getPenetrationPercent() {
  return map(encoderCount, 0, 24, 10, 90);
}

// Converts penetration percent to target height
float getTargetHeightMM() {
  return DECK_HEIGHT_MM * (getPenetrationPercent() / 100.0);
}

// Convert height to stepper steps
int getStepsNeeded() {
  return (int)(getTargetHeightMM() * STEPS_PER_MM);
}


void setup() {
  Serial.begin(115200);

  // sensor and servo setups
  pinMode(SENSOR_1_PIN, INPUT);
  pinMode(SENSOR_2_PIN, INPUT);
  servo1.attach(SERVO_1_PIN);
  servo2.attach(SERVO_2_PIN);
  servo1.writeMicroseconds(servo1Stop);
  servo2.writeMicroseconds(servo2Stop);

  // Encoder setup 
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderISR, FALLING);

  //Stepper setup 
  pinMode(EN_PIN,   OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN,  OUTPUT);
  digitalWrite(EN_PIN,   LOW);   // enable driver
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN,  LOW);

  Serial.println("========================================");
  Serial.println("   CARD DISPENSER SYSTEM: INITIALIZED  ");
  Serial.println("========================================");
  Serial.println("   SUBSYSTEM 3: ENCODER + STEPPER READY");
  Serial.println("========================================");
}


void loop() {


  int state1 = digitalRead(SENSOR_1_PIN);
  int state2 = digitalRead(SENSOR_2_PIN);

  // Tray 1
  if (state1 == LOW) {
    servo1.writeMicroseconds(2000);
    if (lastState1 == HIGH) {
      count1++;
      Serial.print("[TRAY 1] DISPENSING | Total: ");
      Serial.println(count1);
    }
  } else {
    servo1.writeMicroseconds(servo1Stop);
  }
  lastState1 = state1;

  // Tray 2
  if (state2 == LOW) {
    servo2.writeMicroseconds(1000);
    if (lastState2 == HIGH) {
      count2++;
      Serial.print("[TRAY 2] DISPENSING | Total: ");
      Serial.println(count2);
    }
  } else {
    servo2.writeMicroseconds(servo2Stop);
  }
  lastState2 = state2;

  // encoder and stepper
  if (encoderCount != lastEncoderCount) {
    lastEncoderCount = encoderCount;

    float pct    = getPenetrationPercent();
    float height = getTargetHeightMM();
    int   steps  = getStepsNeeded();

    Serial.print("[ENCODER] Count: ");
    Serial.print(encoderCount);
    Serial.print(" | Penetration: ");
    Serial.print(pct, 1);
    Serial.print("% | Height: ");
    Serial.print(height, 1);
    Serial.print("mm | Steps: ");
    Serial.println(steps);

    // Move stepper to match new encoder position
    // TRUE = up, FALSE = down — swap if direction is wrong
    stepMotor(steps, true);
  }

  delay(50);
}

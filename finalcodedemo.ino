#include <ESP32Servo.h>

//PIN DEFINITIONS
// Inputs
const int PB_SHUFFLE   = 2;   // Start Shuffling Button
const int IR_SRV1      = 12;  // IR Forward Line
const int IR_SRV2      = 13;  // IR Reverse Line
const int IR_LIN       = 15;  // IR Linear Actuator Sensor

// Encoder & Stepper
const int BTN_STEP_EN  = 3;   // Encoder Enable Button
const int ENC_A        = 6;
const int ENC_B        = 7;
const int STEP_PIN     = 5;
const int DIR_PIN      = 4;
const int EN_PIN       = 1;   // Stepper Enable (LOW = on, HIGH = off)

// Outputs
const int SRV1         = 18;
const int SRV2         = 19;
const int IN1          = 20;  // Linear Actuator (pos)
const int IN2          = 21;  // Linear Actuator (neg)

// Calibrate
const int STEPS_FOR_FULL_ROTATION = 20; // Adjust for microstepping

// timing adjust
const unsigned long CARD_EMPTY_WAIT = 5000; // 5s wait after cards gone before stopping servos

// --- STATE MACHINE ---
enum SystemState {
  IDLE,
  SHUFFLING,
  WAITING_CARDS_EMPTY,
  ACTUATING_OUT,
  ACTUATING_IN
};
SystemState currentState = IDLE;

// vars
Servo myServo1;
Servo myServo2;

volatile bool encoderTriggered = false;
bool controlMode      = false;
bool trayextDN        = false;
bool lastStepBtnState = HIGH;

unsigned long cardEmptyStartTime = 0;

//INTERRUPT
void IRAM_ATTR readEncoder() {
  encoderTriggered = true;
}

//
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Card Shuffler Initializing...");

  pinMode(PB_SHUFFLE,   INPUT);
  pinMode(BTN_STEP_EN,  INPUT_PULLUP);
  pinMode(IR_LIN,       INPUT);
  pinMode(IR_SRV1,      INPUT);
  pinMode(IR_SRV2,      INPUT);
  pinMode(IN1,          OUTPUT);
  pinMode(IN2,          OUTPUT);
  pinMode(STEP_PIN,     OUTPUT);
  pinMode(DIR_PIN,      OUTPUT);
  pinMode(EN_PIN,       OUTPUT);
  pinMode(ENC_A,        INPUT_PULLUP);
  pinMode(ENC_B,        INPUT_PULLUP);

  digitalWrite(EN_PIN, HIGH); // Stepper OFF at start
  digitalWrite(IN1,    LOW);  // Actuator stopped
  digitalWrite(IN2,    LOW);

  myServo1.attach(SRV1);
  myServo2.attach(SRV2);
  myServo1.writeMicroseconds(1500);
  myServo2.writeMicroseconds(1500);

  attachInterrupt(digitalPinToInterrupt(ENC_A), readEncoder, CHANGE);

  Serial.println("System Ready. Press shuffle button to begin.");
}

// main
void loop() {
  int PBstate   = digitalRead(PB_SHUFFLE);
  int LINstate  = digitalRead(IR_LIN);
  int SRV1state = digitalRead(IR_SRV1);
  int SRV2state = digitalRead(IR_SRV2);


  // ENCODER / STEPPER — runs in every state, always available

  bool currentStepBtn = digitalRead(BTN_STEP_EN);
  if (lastStepBtnState == HIGH && currentStepBtn == LOW) {
    controlMode = !controlMode;
    digitalWrite(EN_PIN, controlMode ? LOW : HIGH);
    Serial.print("Stepper: "); Serial.println(controlMode ? "ON" : "OFF");
    delay(50); // debounce
  }
  lastStepBtnState = currentStepBtn;

  if (controlMode && encoderTriggered) {
    Serial.println("Encoder triggered -> running step");
    runFullRotation();
    encoderTriggered = false;
  } else if (!controlMode) {
    encoderTriggered = false;
  }
 

  //STATE 0: IDLE
  if (currentState == IDLE) {
    if (PBstate == HIGH) {
      Serial.println("IDLE -> SHUFFLING");
      currentState = SHUFFLING;
    }
  }

  //STATE 1: SHUFFLING
  else if (currentState == SHUFFLING) {
    if (SRV1state == LOW || SRV2state == LOW) {
      myServo1.writeMicroseconds(1700);
      myServo2.writeMicroseconds(1300);
    } else {
      Serial.println("Both IR clear. Starting 5s empty confirmation...");
      cardEmptyStartTime = millis();
      currentState = WAITING_CARDS_EMPTY;
    }
  }

  //STATE 2: WAITING FOR CARDS EMPTY CONFIRMATION (5 seconds)
  else if (currentState == WAITING_CARDS_EMPTY) {
    if (SRV1state == LOW || SRV2state == LOW) {
      Serial.println("Card detected again -> back to SHUFFLING");
      currentState = SHUFFLING;
    } else if (millis() - cardEmptyStartTime >= CARD_EMPTY_WAIT) {
      myServo1.writeMicroseconds(1500);
      myServo2.writeMicroseconds(1500);
      Serial.println("Shuffling done -> ACTUATING OUT");
      digitalWrite(EN_PIN, HIGH); // Force stepper off when moving to actuator

      delay(8000);
      currentState = ACTUATING_OUT;
    }
  }

  //STATE 3: ACTUATING OUT
  else if (currentState == ACTUATING_OUT) {
    if (LINstate == LOW && !trayextDN) {
      delay(250);
      extendActuator();
      Serial.println("Actuator extended. Waiting for tray retrieval...");
    } else if (trayextDN && LINstate == HIGH) {
      Serial.println("Tray empty. Waiting 1s then retracting...");
      delay(1000);
      currentState = ACTUATING_IN;
    } else if (LINstate == HIGH && !trayextDN) {
      Serial.println("No cards in collection tray. Skipping actuator.");
      currentState = ACTUATING_IN;
    }
  }

  //STATE 4: ACTUATING IN / RESET
  else if (currentState == ACTUATING_IN) {
    retractActuator();
    trayextDN = false;
    Serial.println("RESET COMPLETE -> IDLE");
    currentState = IDLE;
  }
}


void runFullRotation() {
  digitalWrite(DIR_PIN, HIGH);
  for (int i = 0; i < STEPS_FOR_FULL_ROTATION; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1000);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000);
  }
}

void extendActuator() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  trayextDN = true;
}

void retractActuator() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

void stopActuator() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}
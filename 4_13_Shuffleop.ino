#include <ESP32Servo.h>

const int PB = 2;        //Pushbutton input
const int IR_SRV1 = 12;  //SRV Motor Line 1 IR (forward line)
const int IR_SRV2 = 13;  //SRV Motor Line 2 IR (reveerse line)
const int IR_LIN = 15;    //Linear Actuator IR Sensor

const int SRV1 = 18;     //Servo Motor Line 1
const int SRV2 = 19;     //Servo Motor Line 2
const int IN1 = 20;      //Linear Actuator Input (White)
const int IN2 = 21;      //Linear Actuator Input (Brown)

Servo myServo1;         //Create servo line 1 class
Servo myServo2;         //Create servo line 2 class
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32-C6 is working");
  pinMode(PB, INPUT);
  pinMode(IR_LIN, INPUT);
  pinMode(IR_SRV1, INPUT);
  pinMode(IR_SRV2, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  myServo1.attach(SRV1);
  myServo2.attach(SRV2);
  

  // stop actuator at startup
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

}
bool shuffleEN = false;
bool trayextDN = false;
void loop() {
  //Serial.println("test");
  int PBstate = digitalRead(PB);
  int LINstate = digitalRead(IR_LIN);
  int SRV1state = digitalRead(IR_SRV1);     //forward line
  int SRV2state = digitalRead(IR_SRV2);     //reverse line
  

  //delay(500);
if (PBstate == HIGH) {
    //delay(500);
  shuffleEN = true;
   // Serial.println("high");}
}else //{Serial.println("low");} 
  
if(shuffleEN == true) {
  Serial.println("GO");
  // Card Detection
   if ((SRV1state == LOW) || (SRV2state == LOW)){
   // Serial.println("CARD DETECT");
    myServo1.writeMicroseconds(1700);
    myServo2.writeMicroseconds(1300); }
    else { 
    //  Serial.println("NO SRV CARD DETECT");
      myServo1.writeMicroseconds(1500);
      myServo2.writeMicroseconds(1500);}
 
 
//Test code 
/*
if (PBstate == HIGH)
{
  extendActuator();   
}
  else {retractActuator();} */
  
  

  if (LINstate == LOW && SRV1state == HIGH && SRV2state == HIGH) //Only run if collection tray cards are present and Pre-shuffle trays are both empty
  {
    delay(250);            //Slight delay to ensure all cards have fallen to collection tray
    extendActuator();
    //Serial.println("LIN - Object detected -> Extending actuator");
  } else if(trayextDN == true) {
    delay(2000);           //Delay to minimize human pinchpoints
    retractActuator();     //Retract tray after user retrieves decks
   // Serial.println("LIN - No object -> Actuator stopped");
  }
}
}
void extendActuator() {         //Extend Collection tray
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  trayextDN = true;             //Changes value after tray if fully extended (enables delay for retraction) 
}

void retractActuator() {        //Retract empty collection tray
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

void stopActuator() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW); 
}

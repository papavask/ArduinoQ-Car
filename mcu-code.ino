#include <Arduino_RouterBridge.h>

// Lafvin DC Motor Pins
const int dirRight = 2;
const int pwmRight = 5;
const int dirLeft = 4;
const int pwmLeft = 6;

// Lafvin HC-SR04 Ultrasonic Pins
const int TRIG_PIN = 12;
const int ECHO_PIN = 13;
const int OBSTACLE_LIMIT_CM = 15; // Stop if obstacle is closer than 15cm

// Flags for Bridge execution
volatile bool pendingMove = false;
volatile float pendingSeconds = 1.0f;
volatile int pendingSpeed = 150;
char pendingCmd[16] = "";

// Ultrasonic logic adapted from Lafvin checkdistance()
float readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  float distance = pulseIn(ECHO_PIN, HIGH, 30000) / 58.00; // 30ms timeout
  delay(10);
  return distance;
}

void stopMotors() {
  digitalWrite(dirRight, LOW);
  analogWrite(pwmRight, 0);
  digitalWrite(dirLeft, HIGH);
  analogWrite(pwmLeft, 0);
}

void doForward(float seconds, int speed) {
  digitalWrite(dirRight, HIGH);
  analogWrite(pwmRight, speed);
  digitalWrite(dirLeft, LOW);
  analogWrite(pwmLeft, speed);
  
  unsigned long start = millis();
  unsigned long duration = (unsigned long)(seconds * 1000.0f);
  
  // Smart Delay: Poll the HC-SR04 sensor rapidly while moving
  while (millis() - start < duration) {
    float dist = readUltrasonic();
    
    // dist > 0 prevents false 0 readings on timeouts
    if (dist > 0 && dist <= OBSTACLE_LIMIT_CM) {
      Serial.print("Obstacle at ");
      Serial.print(dist);
      Serial.println(" cm! Emergency stop.");
      break; 
    }
  }
  
  stopMotors();
}

void doBack(float seconds, int speed) {
  digitalWrite(dirRight, LOW);
  analogWrite(pwmRight, speed);
  digitalWrite(dirLeft, HIGH);
  analogWrite(pwmLeft, speed);
  delay((unsigned long)(seconds * 1000.0f));
  stopMotors();
}

void doRight(float seconds, int speed) {
  digitalWrite(dirRight, HIGH);
  analogWrite(pwmRight, speed);
  digitalWrite(dirLeft, HIGH);
  analogWrite(pwmLeft, speed);
  delay((unsigned long)(seconds * 1000.0f));
  stopMotors();
}

void doLeft(float seconds, int speed) {
  digitalWrite(dirRight, LOW);
  analogWrite(pwmRight, speed);
  digitalWrite(dirLeft, LOW);
  analogWrite(pwmLeft, speed);
  delay((unsigned long)(seconds * 1000.0f));
  stopMotors();
}

void handleMove(String cmd, float seconds, int speed) { 
  strncpy(pendingCmd, cmd.c_str(), sizeof(pendingCmd) - 1);
  pendingSeconds = seconds;
  pendingSpeed = speed;
  pendingMove = true;
}

void setup() {
  Serial.begin(115200);
  
  pinMode(dirRight, OUTPUT);
  pinMode(pwmRight, OUTPUT);
  pinMode(dirLeft, OUTPUT);
  pinMode(pwmLeft, OUTPUT);
  
  // Initialize Ultrasonic pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  stopMotors();

  if (!Bridge.begin()) {
    Serial.println("Bridge start fail");
  }
  Bridge.provide_safe("move", handleMove);
  Serial.println("Bridge ready.");
}

void loop() {
  if (pendingMove) {
    pendingMove = false;
    String cmd = String(pendingCmd);
    
    if (cmd == "forward") doForward(pendingSeconds, pendingSpeed);
    else if (cmd == "back") doBack(pendingSeconds, pendingSpeed);
    else if (cmd == "right") doRight(pendingSeconds, pendingSpeed);
    else if (cmd == "left") doLeft(pendingSeconds, pendingSpeed);
    else if (cmd == "stop") stopMotors();
  }
  delay(10);
}

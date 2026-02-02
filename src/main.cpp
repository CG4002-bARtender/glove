#include <Arduino.h>
#include <vector>

// Constants
constexpr int flexPins[5] = { A0, A1, A2, A3, A4 };
constexpr int flexThreshold = 3600;

void setup() {
  Serial.begin(9600);

  for (const int flexPin : flexPins){
    pinMode(flexPin, INPUT);
  }
}

void loop() {
  for (const int flexPin : flexPins){
    readFlexPin(flexPin);
  }
}

void readFlexPin(const int flexPin) {
  int flexValue;
  flexValue = analogRead(flexPin);
  
  bool isOverThreshold = flexValue > flexThreshold;

  printf("Flex Pin: %d | Raw Value: %d | Flexed: %s", flexPin, flexValue, flexThreshold ? "YES" : "NO"); 
}
#include "Sensors.h"

Sensors::Sensors() {}

void Sensors::begin() {
  setupPins();
}

void Sensors::setupPins() {
  pinMode(Door_Sensor, INPUT);   // HAS HARDWARE PULLUP
  pinMode(Water_Sensor, INPUT_PULLUP);
  pinMode(Smoke_Sensor, INPUT);  // HAS HARDWARE PULLUP
}

bool Sensors::isDoorOpen() {
  return digitalRead(Door_Sensor) == HIGH;
}

bool Sensors::isWaterPresent() {
  return digitalRead(Water_Sensor) == LOW;
  return false;
}

bool Sensors::isSmokeDetected() {
  return digitalRead(Smoke_Sensor) == LOW;
  return false;
}


// bool Sensors::isSmokeDetected() {
//   bool smokeDetected = false;
//   bool currentSmokeState = digitalRead(Smoke_Sensor) == LOW;

//   if (currentSmokeState) {
//     if (!smokeDetected) {
//       // Smoke detected for the first time, record the start time
//       smokeDetectionStartTime = millis();
//       smokeDetected = true;
//     } else {
//       // Smoke is still detected, check if the threshold has been reached
//       if (millis() - smokeDetectionStartTime >= smokeDetectionTime) {
//         return true;  // Smoke detected for at least 2 seconds
//       }
//     }
//   } else {
//     // Smoke is not detected, reset the state
//     smokeDetected = false;
//   }

//   return false;  // Smoke not detected or not detected for long enough
// }


float Sensors::readInputVoltage(int adcPin, int sampleCount = 10) {
  float total = 0.0;
  for (int i = 0; i < sampleCount; i++) {
    int adcValue = analogRead(adcPin);
    total += adcValue;
    // delay(5);

    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
  float averageAdcValue = total / sampleCount;
  float voltage = (averageAdcValue * ADC_MAX_VOLTAGE) / STM32_ADC_RESOLUTION * VOLTAGE_GAIN;
  if (voltage < 5) {
    voltage = 5;
  }
  return 48;
  // return voltage;
}

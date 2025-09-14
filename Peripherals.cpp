#include "Peripherals.h"

Peripherals::Peripherals() {}

void Peripherals::begin() {
  setupPins();
  controlLight(false);
}

void Peripherals::setupPins() {
  pinMode(CA_LIGHT_PIN, OUTPUT);
  pinMode(RELAY_1_Trigger, OUTPUT);
  pinMode(RELAY_2_Trigger, OUTPUT);
  pinMode(RELAY_3_Trigger, OUTPUT);
  pinMode(RELAY_4_Trigger, OUTPUT);
  pinMode(RELAY_5_Trigger, OUTPUT);
  pinMode(BUZZER_Trigger, OUTPUT);
}

void Peripherals::triggerRelay(int relay, bool state) {
  digitalWrite(relay, state ? HIGH : LOW);
}

void Peripherals::controlLight(bool state) {
  digitalWrite(CA_LIGHT_PIN, state ? HIGH : LOW);
}

void Peripherals::alertBuzzer(int count) {
  unsigned long previousMillis = 0;
  const unsigned long onDuration = 400;
  const unsigned long offDuration = 300;

  for (int i = 0; i < count; i++) {
    // Turn on buzzer
    digitalWrite(BUZZER_Trigger, HIGH);
    previousMillis = millis();
    while (millis() - previousMillis <= onDuration) {
      // Wait for onDuration
      yield();  // Optional: helps multitasking in ESP-based boards
    }

    // Turn off buzzer
    digitalWrite(BUZZER_Trigger, LOW);
    previousMillis = millis();
    while (millis() - previousMillis <= offDuration) {
      // Wait for offDuration
      yield();
    }
  }
}

// Software PWM function for STM32 without analogWrite()
// Blocking version

void softPWM(uint8_t pin, uint8_t dutyCycle, uint32_t freqHz, uint32_t durationMs) {
  pinMode(pin, OUTPUT);

  uint32_t periodMicrosec = 1000000UL / freqHz;          // Total period in microseconds
  uint32_t onTime = (periodMicrosec * dutyCycle) / 100;  // High time
  uint32_t offTime = periodMicrosec - onTime;            // Low time

  uint32_t cycles = (durationMs * 1000UL) / periodMicrosec;  // How many periods in the duration

  for (uint32_t i = 0; i < cycles; i++) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(onTime);
    digitalWrite(pin, LOW);
    delayMicroseconds(offTime);
  }
}

void buzzSimple(uint8_t pin, uint8_t beepCount = 1, uint16_t onTimeMs = 100, uint16_t offTimeMs = 100) {
  pinMode(pin, OUTPUT);
  for (uint8_t i = 0; i < beepCount; i++) {
    digitalWrite(pin, HIGH);
    delay(onTimeMs);
    digitalWrite(pin, LOW);
    if (i < beepCount - 1) delay(offTimeMs);
  }
}


void Peripherals::confirmBeep() {
  // softPWM(BUZZER_Trigger, 100, 100, 40);  // 100Hz tone, 40ms duration
  buzzSimple(BUZZER_Trigger, 2, 40, 40);  // 2 quick beeps, 40ms on, 40ms off
}

void Peripherals::buttonPressFeedback() {
  static unsigned long buzzStart = 0;
  static bool buzzing = false;

  softPWM(BUZZER_Trigger, 100, 1000, 10);

  // digitalWrite(BUZZER_Trigger, HIGH);
  // delay(10);
  // digitalWrite(BUZZER_Trigger, LOW);
  // delay(10);
}

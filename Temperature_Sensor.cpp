#include "Temperature_Sensor.h"

TemperatureSensor::TemperatureSensor()
  : oneWire(Temp_Sensor), sensors(&oneWire) {}

void TemperatureSensor::begin() {
  setupPins();
  sensors.begin();
}

void TemperatureSensor::setupPins() {
  // pinMode(Temp_Sensor, INPUT_PULLUP); // already has hardware pullup
}

float TemperatureSensor::readTemperature() {
  if (sensors.getDeviceCount() == 0) {
    DEBUG_PRINTLN("No temperature sensor detected.");
    reinitializeSensor();
    // return DEVICE_DISCONNECTED_C;
    return TEMP_HIGH_THRESHOLD + 11;  // Return high threshold to trigger alarm
  }

  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);

  if (temperature == DEVICE_DISCONNECTED_C) {
    DEBUG_PRINTLN("Sensor read failed. Attempting to reinitialize...");
    reinitializeSensor();
    return TEMP_HIGH_THRESHOLD + 11;  // Treat read failure same as missing sensor
  }

  return temperature;
}

void TemperatureSensor::reinitializeSensor() {
  unsigned long now = millis();

  // Avoid spamming reinitialization
  if (now - lastReconnectAttempt > 5000) {  // 5 seconds delay
    lastReconnectAttempt = now;
    sensors.begin();  // reinitialize DallasTemperature
    DEBUG_PRINTLN("Reinitialized temperature sensor");
  }
}

#ifndef SENSORS_H
#define SENSORS_H

#include "ODCC_config.h"
#include <STM32FreeRTOS.h>


class Sensors {
public:
  Sensors();
  void begin();
  bool isDoorOpen();
  bool isWaterPresent();
  bool isSmokeDetected();
  float readInputVoltage(int adcPin, int sampleCount);

private:
  void setupPins();
};

#endif  // SENSORS_H

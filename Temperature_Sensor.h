#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include "ODCC_config.h"
#include "Fan_Controller.h"
#include <OneWire.h>
#include <DallasTemperature.h>

class TemperatureSensor {
public:
    TemperatureSensor();
    void begin();
    float readTemperature();
    void setupPins();

private:
    OneWire oneWire;
    DallasTemperature sensors;
    unsigned long lastReconnectAttempt = 0;
    void reinitializeSensor();
};

#endif // TEMPERATURE_SENSOR_H

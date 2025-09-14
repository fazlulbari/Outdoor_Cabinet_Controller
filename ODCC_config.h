#ifndef ODCC_CONFIG_H
#define ODCC_CONFIG_H

#include "MCU_config.h"

extern const float ADC_MAX_VOLTAGE;
extern const int STM32_ADC_RESOLUTION;
extern const float VOLTAGE_GAIN;


// Baud rate options
extern uint32_t BAUD_RATES[];
extern int NUM_BAUD_RATES;

// ---------------------- System Parameters ----------------------
extern float temperature;
extern float inputVoltage;

// ------------------ Voltage Threshold Values -------------------
extern int VOLTAGE_LOW_THRESHOLD;
extern int VOLTAGE_HIGH_THRESHOLD;


// ----------------------- Alarm Parameters ----------------------
extern uint16_t ALARM_DELAY;
extern uint16_t DOOR_ALARM_DELAY;
extern uint16_t FAN_FAULT_ALARM_DELAY;


// Define discrete input indices for alarms
enum HoldingRegIndices {
  REG_LOWER_THRESHOLD = 1,
  REG_UPPER_THRESHOLD,
  REG_HIGH_THRESHOLD,
  REG_TOGGLE_PERIOD,
  REG_FAN_TYPE,
  REG_FAN_PROFILE,
  REG_CUSTOM_MAX_FAN_SPEED,
  
  // REG for fan fault threshold control
  REG_DF_OPEN_THRESHOLD,
  REG_SF_OPEN_THRESHOLD,
  REG_SF_JAM_THRESHOLD,
  REG_DF_JAM_THRESHOLD,
  REG_SF_SHORT_THRESHOLD,
  REG_DF_SHORT_THRESHOLD,
  

  REG_VOLTAGE_LOW,
  REG_VOLTAGE_HIGH,

  REG_Slave_id,
  REG_Baud_Rate,

  HOLDING_REG_MAX  // Automatically adjusts when new entries are added
};



// Define input register indices using an enumeration
enum InputRegisterIndices {
  REG_INPUT_VOLTAGE = 1,
  REG_TEMPERATURE,


  REG_BAUD_RATE,
  REG_SLAVE_ID,

  REG_MAX  // Automatically adjusts when new entries are added
};


// Define discrete input indices for alarms
enum DiscreteInputIndices {
  ALARM_DOOR_OPEN = 1,
  ALARM_WATER_INGRES,
  ALARM_SMOKE_DETECTED,
  ALARM_HIGH_TEMPERATURE,
  ALARM_FAN_FAULT,

  ALARM_MAX  // Automatically adjusts when new entries are added
};


// Define global Modbus registers dynamically sized
extern bool coils[10];
extern bool discreteInputs[ALARM_MAX];
extern uint16_t inputRegisters[REG_MAX];
extern uint16_t holdingRegisters[HOLDING_REG_MAX];
extern uint16_t old_HoldingRegisters[HOLDING_REG_MAX];


extern bool Smoke_detected_alarm;
extern bool High_temperature_alarm;
extern bool Water_ingres_alarm;
extern bool Door_open_alarm;
extern bool Fan_fault_alarm;
extern bool isAlarmActive;

// Cabinet Power voltage sense
#define VOLTAGE_ADC_PIN PA0  // Analog pin ADC for Volatge sensing

// Main control pins
#define FANPORT1_PWM_PIN PA7
#define FANPORT1_SHUNT_PIN PA5  // Analog pin ADC for Current sensing

#define FANPORT2_PWM_PIN PA6
#define FANPORT2_SHUNT_PIN PA4  // Analog pin ADC for Current sensing

#define CA_LIGHT_PIN PA12  // Cabinet light, Turn ON when Door opens

// Relay Alarm pins
#define DOOR_ALARM RELAY_1_Trigger
#define WATER_ALARM RELAY_2_Trigger
#define SMOKE_ALARM RELAY_3_Trigger
#define HIGH_TEMP_ALARM RELAY_4_Trigger
#define FAN_FAULT_ALARM RELAY_5_Trigger
#define BUZZER_Trigger RELAY_6_Trigger


// Digital Sensors
#define Door_Sensor PB9   //  Use Input PullUp, 0 when Opens
#define Water_Sensor PB1  //  Use Input PullUp, 0 when Water enters
#define Smoke_Sensor PB8  //  Use Input PullUp, 0 when smoke detected, also pulses every 5-6 seconds

// Temperature sensor
#define Temp_Sensor PC13  // DS18B20 Temperature Sensor, Onewire protocol

// Temperature threshold



#endif  // ODCC_CONFIG_H

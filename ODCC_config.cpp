#include "ODCC_config.h"

const float ADC_MAX_VOLTAGE = 3.3;
const int STM32_ADC_RESOLUTION = 4095;
const float VOLTAGE_GAIN = 100;


// ---------------------- System Parameters ----------------------
float temperature = 30.0;
float inputVoltage = 48.0;

// ------------------ Voltage Threshold Values -------------------
int VOLTAGE_LOW_THRESHOLD  = 36;   // Volts
int VOLTAGE_HIGH_THRESHOLD = 57;   // Volts

// ----------------------- Alarm Parameters ----------------------
uint16_t ALARM_DELAY           = 2000;   // 2 seconds
uint16_t DOOR_ALARM_DELAY      = 10000;  // 10 seconds
uint16_t FAN_FAULT_ALARM_DELAY = 45000;  // 45 seconds



// Define global Modbus registers dynamically sized
uint16_t inputRegisters[REG_MAX] = { 0 };
bool discreteInputs[ALARM_MAX] = { 0 };

bool coils[10];
uint16_t holdingRegisters[HOLDING_REG_MAX];
uint16_t old_HoldingRegisters[HOLDING_REG_MAX];


// Baud rate options
uint32_t BAUD_RATES[] = { 9600, 19200, 38400, 57600, 115200 };
int NUM_BAUD_RATES = sizeof(BAUD_RATES) / sizeof(BAUD_RATES[0]);

// Flags
bool Smoke_detected_alarm = false;
bool High_temperature_alarm = false;
bool Water_ingres_alarm = false;
bool Door_open_alarm = true;  // initialize with door open
bool Fan_fault_alarm = false;

bool isAlarmActive = false;

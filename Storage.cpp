#include "Storage.h"
#include <FlashStorage_STM32F1.h>
#include "modbus.h"

#define USING_FLASH_SECTOR_NUMBER (REGISTERED_NUMBER_FLASH_SECTORS - 2)

// ------------------------EEPROM Address ------------------------
uint8_t SLAVE_ID_ADDR = 0;
uint8_t BAUD_RATE_INDEX_ADDR = 2;
uint8_t LOWER_TEMP_THRESHOLD_ADDR = 4;
uint8_t UPPER_TEMP_THRESHOLD_ADDR = 6;
uint8_t TEMP_HIGH_THRESHOLD_ADDR = 8;
uint8_t TOGGLE_PERIOD_ADDR = 10;
uint8_t VOLTAGE_LOW_THRESHOLD_ADDR = 12;
uint8_t VOLTAGE_HIGH_THRESHOLD_ADDR = 14;
uint8_t FAN_MODEL_ADDR = 16;
uint8_t FAN_MODE_ADDR = 18;

uint8_t CUSTOM_MAX_FAN_SPEED_ADDR = 20;

uint8_t DOUBLE_FAN_OPEN_THRESHOLD_ADDR   = 22;
uint8_t SINGLE_FAN_OPEN_THRESHOLD_ADDR   = 24;
uint8_t SINGLE_FAN_JAM_THRESHOLD_ADDR    = 26;
uint8_t DOUBLE_FAN_JAM_THRESHOLD_ADDR    = 28;
uint8_t SINGLE_FAN_SHORT_THRESHOLD_ADDR  = 30;
uint8_t DOUBLE_FAN_SHORT_THRESHOLD_ADDR  = 32;


void Storage::begin() {
  DEBUG_PRINTLN(FLASH_STORAGE_STM32F1_VERSION);
  DEBUG_PRINT("EEPROM length: ");
  DEBUG_PRINTLN(EEPROM.length());
  EEPROM.init();
}

int Storage::read(int address) {
  return EEPROM.read(address);
}

int Storage::read16(int baseAddress) {
  int low = EEPROM.read(baseAddress);
  int high = EEPROM.read(baseAddress + 1);
  return (high << 8) | low;
}

void Storage::update(int address, int value) {
  int previous = read(address);
  if (value != previous) {
    EEPROM.update(address, value);
    EEPROM.commit();
    DEBUG_PRINT("Updated 16-bit value at ");
    DEBUG_PRINT(address);
    DEBUG_PRINT(" = ");
    DEBUG_PRINTLN(value);
  } else {
    DEBUG_PRINTLN("Value Already There!");
  }
}

void Storage::update16(int baseAddress, int value) {
  int previous = read16(baseAddress);
  if (value != previous) {
    EEPROM.write(baseAddress, value & 0xFF);
    EEPROM.write(baseAddress + 1, (value >> 8) & 0xFF);
    EEPROM.commit();
    DEBUG_PRINT("Updated 16-bit value at ");
    DEBUG_PRINT(baseAddress);
    DEBUG_PRINT(" = ");
    DEBUG_PRINTLN(value);
  } else {
    DEBUG_PRINTLN("Value Already There!");
  }
}

// void Storage::clear(int address) {
//   EEPROM.write(address, 0);
//   EEPROM.commit();
// }

// void Storage::format() {
//   for (int i = 0; i < EEPROM.length(); i++) {
//     EEPROM.write(i, 0);
//   }
//   EEPROM.commit();
// }

void Storage::loadModbusRegisters() {
  int eepRom_Addr = 50;
  Serial.println("");
  for (int i = 0; i < HOLDING_REG_MAX; i++) {
    Serial.println("Loading data from EEP rom");
    holdingRegisters[i] = read(eepRom_Addr);
    Serial.print(holdingRegisters[i]);
    Serial.print(" | ");
    eepRom_Addr++;
  }
}

void Storage::loadFromEEPROM() {
  LOWER_TEMP_THRESHOLD     = read(LOWER_TEMP_THRESHOLD_ADDR);
  UPPER_TEMP_THRESHOLD     = read(UPPER_TEMP_THRESHOLD_ADDR);
  TEMP_HIGH_THRESHOLD      = read(TEMP_HIGH_THRESHOLD_ADDR);
  TOGGLE_PERIOD            = read(TOGGLE_PERIOD_ADDR);
  Slave_id                 = read(SLAVE_ID_ADDR);
  CUSTOM_MAX_FAN_SPEED     = read(CUSTOM_MAX_FAN_SPEED_ADDR);
  uint8_t baudRateIndex    = read(BAUD_RATE_INDEX_ADDR);
  uint8_t fanModelIndex    = read(FAN_MODEL_ADDR);
  uint8_t fanModeIndex     = read(FAN_MODE_ADDR);
  VOLTAGE_HIGH_THRESHOLD   = read(VOLTAGE_HIGH_THRESHOLD_ADDR);
  VOLTAGE_LOW_THRESHOLD    = read(VOLTAGE_LOW_THRESHOLD_ADDR);

  Baud_Rate   = BAUD_RATES[baudRateIndex];
  FAN_TYPE    = FAN_MODEL[fanModelIndex];
  FAN_PROFILE = FAN_MODE[fanModeIndex];

  SINGLE_FAN_OPEN_CURRENT_THRESHOLD   = read16(SINGLE_FAN_OPEN_THRESHOLD_ADDR);
  DOUBLE_FAN_OPEN_CURRENT_THRESHOLD   = read16(DOUBLE_FAN_OPEN_THRESHOLD_ADDR);
  SINGLE_FAN_JAM_CURRENT_THRESHOLD    = read16(SINGLE_FAN_JAM_THRESHOLD_ADDR);
  DOUBLE_FAN_JAM_CURRENT_THRESHOLD    = read16(DOUBLE_FAN_JAM_THRESHOLD_ADDR);
  SINGLE_FAN_SHORT_CURRENT_THRESHOLD  = read16(SINGLE_FAN_SHORT_THRESHOLD_ADDR);
  DOUBLE_FAN_SHORT_CURRENT_THRESHOLD  = read16(DOUBLE_FAN_SHORT_THRESHOLD_ADDR);

  holdingRegisters[REG_LOWER_THRESHOLD]       = LOWER_TEMP_THRESHOLD;
  holdingRegisters[REG_UPPER_THRESHOLD]       = UPPER_TEMP_THRESHOLD;
  holdingRegisters[REG_HIGH_THRESHOLD]        = TEMP_HIGH_THRESHOLD;
  holdingRegisters[REG_TOGGLE_PERIOD]         = TOGGLE_PERIOD;
  holdingRegisters[REG_CUSTOM_MAX_FAN_SPEED]  = CUSTOM_MAX_FAN_SPEED;
  holdingRegisters[REG_FAN_TYPE]              = fanModelIndex;
  holdingRegisters[REG_FAN_PROFILE]           = fanModeIndex;


  holdingRegisters[REG_DF_OPEN_THRESHOLD]     = DOUBLE_FAN_OPEN_CURRENT_THRESHOLD;
  holdingRegisters[REG_SF_OPEN_THRESHOLD]     = SINGLE_FAN_OPEN_CURRENT_THRESHOLD;
  holdingRegisters[REG_SF_JAM_THRESHOLD]      = SINGLE_FAN_JAM_CURRENT_THRESHOLD;
  holdingRegisters[REG_DF_JAM_THRESHOLD]      = DOUBLE_FAN_JAM_CURRENT_THRESHOLD;
  holdingRegisters[REG_SF_SHORT_THRESHOLD]    = SINGLE_FAN_SHORT_CURRENT_THRESHOLD;
  holdingRegisters[REG_DF_SHORT_THRESHOLD]    = DOUBLE_FAN_SHORT_CURRENT_THRESHOLD;

  holdingRegisters[REG_VOLTAGE_HIGH]          = VOLTAGE_HIGH_THRESHOLD;
  holdingRegisters[REG_VOLTAGE_LOW]           = VOLTAGE_LOW_THRESHOLD;

  holdingRegisters[REG_Slave_id]              = Slave_id;
  holdingRegisters[REG_Baud_Rate]             = baudRateIndex;
  
}

#ifndef STORAGE_H
#define STORAGE_H

#include "MCU_config.h"
#include "ODCC_config.h"
#include "Fan_Controller.h"


// ------------------------EEPROM Address ------------------------
extern uint8_t SLAVE_ID_ADDR;
extern uint8_t BAUD_RATE_INDEX_ADDR;
extern uint8_t LOWER_TEMP_THRESHOLD_ADDR;
extern uint8_t UPPER_TEMP_THRESHOLD_ADDR;
extern uint8_t TEMP_HIGH_THRESHOLD_ADDR;
extern uint8_t TOGGLE_PERIOD_ADDR;
extern uint8_t VOLTAGE_LOW_THRESHOLD_ADDR;
extern uint8_t VOLTAGE_HIGH_THRESHOLD_ADDR;
extern uint8_t FAN_MODEL_ADDR;
extern uint8_t FAN_MODE_ADDR;

extern uint8_t CUSTOM_MAX_FAN_SPEED_ADDR;

extern uint8_t DOUBLE_FAN_OPEN_THRESHOLD_ADDR;
extern uint8_t SINGLE_FAN_OPEN_THRESHOLD_ADDR;
extern uint8_t SINGLE_FAN_JAM_THRESHOLD_ADDR;
extern uint8_t DOUBLE_FAN_JAM_THRESHOLD_ADDR;
extern uint8_t SINGLE_FAN_SHORT_THRESHOLD_ADDR;
extern uint8_t DOUBLE_FAN_SHORT_THRESHOLD_ADDR;



class Storage {
public:
  Storage() = default;

  void begin();
  // void clear(int address);
  // void format();
  void update(int address, int value);
  void update16(int address, int value);
  int read(int address);
  int read16(int address);
  void loadModbusRegisters();
  void loadFromEEPROM();
};

#endif  // STORAGE_H

#include "modbus.h"

// Define HardwareSerial and ModbusRTUSlave globally
HardwareSerial mySerial(USART2);
// Define ModbusRTUSlave object
ModbusRTUSlave slave(mySerial);

extern Storage storage;

// ---------------------- Modbus Parameters ----------------------
int Baud_Rate = 9600;
int Slave_id = 11;


// Previous settings for checking changes
static int previousBaudRate = -1;  // Set an invalid initial value
static int previousSlaveId = -1;   // Set an invalid initial value

void save_N_Update_eepRom(int registerAddr, int eepRomAddr);
void modbus_Callback();

void modbus_init() {
  MODBUS_DEBUG_PRINTLN("Initializing Modbus...");
  //load_ModbusReg();
  mySerial.begin(Baud_Rate, SERIAL_8N1);         // Initialize serial communication for RS485
  slave.begin(Slave_id, Baud_Rate, SERIAL_8N1);  // Initialize Modbus slave

  // Configure discrete inputs and input registers with dynamic sizes
  slave.configureDiscreteInputs(discreteInputs, 10);
  slave.configureInputRegisters(inputRegisters, REG_MAX);
  slave.configureHoldingRegisters(holdingRegisters, 20);  // unsigned 16 bit integer array of holding register values, number of holding registers
  slave.configureCoils(coils, ALARM_MAX);                 // bool array of coil values, number of coils

  // Store previous settings to detect changes
  previousBaudRate = Baud_Rate;
  previousSlaveId = Slave_id;

  for (int i = 0; i < HOLDING_REG_MAX; i++) {
    old_HoldingRegisters[i] = holdingRegisters[i];
  }


  MODBUS_DEBUG_PRINTLN("Modbus initialized successfully.");
}


void modbus_loop() {

  // Check if Modbus settings have changed
  if (Baud_Rate != previousBaudRate || Slave_id != previousSlaveId) {
    MODBUS_DEBUG_PRINTLN("Baud rate or Slave ID changed. Reinitializing Modbus...");
    modbus_init();  // Reinitialize Modbus only when needed
  }

  modbus_Callback();

  // Assign values dynamically
  inputRegisters[REG_INPUT_VOLTAGE] = inputVoltage;
  inputRegisters[REG_TEMPERATURE] = temperature;

  inputRegisters[REG_BAUD_RATE] = Baud_Rate;
  inputRegisters[REG_SLAVE_ID] = Slave_id;

  // Assign alarm flags dynamically
  coils[ALARM_SMOKE_DETECTED] = Smoke_detected_alarm;
  coils[ALARM_HIGH_TEMPERATURE] = High_temperature_alarm;
  coils[ALARM_WATER_INGRES] = Water_ingres_alarm;
  coils[ALARM_DOOR_OPEN] = Door_open_alarm;
  coils[ALARM_FAN_FAULT] = Fan_fault_alarm;

  // Poll Modbus slave
  slave.poll();
}


void save_N_Update_eepRom(int registerAddr, int eepRomAddr) {
  if (old_HoldingRegisters[registerAddr] != holdingRegisters[registerAddr]) {

    // If the value is expected to exceed 255 or is part of a 16-bit setting, use 16-bit save
    if (
      // registerAddr == REG_CUSTOM_MAX_FAN_SPEED ||
      registerAddr == REG_SF_OPEN_THRESHOLD || registerAddr == REG_DF_OPEN_THRESHOLD || registerAddr == REG_SF_JAM_THRESHOLD || registerAddr == REG_DF_JAM_THRESHOLD || registerAddr == REG_SF_SHORT_THRESHOLD || registerAddr == REG_DF_SHORT_THRESHOLD) {
      storage.update16(eepRomAddr, holdingRegisters[registerAddr]);
    } else {
      storage.update(eepRomAddr, holdingRegisters[registerAddr]);
    }

    storage.loadFromEEPROM();  // Sync EEPROM → globals + registers
  }

  old_HoldingRegisters[registerAddr] = holdingRegisters[registerAddr];
}

void modbus_Callback() {

  save_N_Update_eepRom(REG_LOWER_THRESHOLD, LOWER_TEMP_THRESHOLD_ADDR);
  save_N_Update_eepRom(REG_UPPER_THRESHOLD, UPPER_TEMP_THRESHOLD_ADDR);
  save_N_Update_eepRom(REG_HIGH_THRESHOLD, TEMP_HIGH_THRESHOLD_ADDR);
  save_N_Update_eepRom(REG_TOGGLE_PERIOD, TOGGLE_PERIOD_ADDR);

  save_N_Update_eepRom(REG_FAN_TYPE, FAN_MODEL_ADDR);
  save_N_Update_eepRom(REG_FAN_PROFILE, FAN_MODE_ADDR);

  save_N_Update_eepRom(REG_CUSTOM_MAX_FAN_SPEED, CUSTOM_MAX_FAN_SPEED_ADDR);
  save_N_Update_eepRom(REG_DF_OPEN_THRESHOLD, DOUBLE_FAN_OPEN_THRESHOLD_ADDR);
  save_N_Update_eepRom(REG_SF_OPEN_THRESHOLD, SINGLE_FAN_OPEN_THRESHOLD_ADDR);
  save_N_Update_eepRom(REG_SF_JAM_THRESHOLD, SINGLE_FAN_JAM_THRESHOLD_ADDR);
  save_N_Update_eepRom(REG_DF_JAM_THRESHOLD, DOUBLE_FAN_JAM_THRESHOLD_ADDR);
  save_N_Update_eepRom(REG_SF_SHORT_THRESHOLD, SINGLE_FAN_SHORT_THRESHOLD_ADDR);
  save_N_Update_eepRom(REG_DF_SHORT_THRESHOLD, DOUBLE_FAN_SHORT_THRESHOLD_ADDR);

  save_N_Update_eepRom(REG_VOLTAGE_LOW, VOLTAGE_LOW_THRESHOLD_ADDR);
  save_N_Update_eepRom(REG_VOLTAGE_HIGH, VOLTAGE_HIGH_THRESHOLD_ADDR);

  save_N_Update_eepRom(REG_Slave_id, SLAVE_ID_ADDR);
  save_N_Update_eepRom(REG_Baud_Rate, BAUD_RATE_INDEX_ADDR);


  MODBUS_DEBUG_PRINT("");
  for (int i = 1; i < HOLDING_REG_MAX; i++) {
    MODBUS_DEBUG_PRINT(holdingRegisters[i]);
    MODBUS_DEBUG_PRINT(" | ");
  }
  MODBUS_DEBUG_PRINTLN("");
}

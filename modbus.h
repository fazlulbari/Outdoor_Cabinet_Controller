#ifndef MODBUS_H
#define MODBUS_H

#include "ODCC_config.h"
#include "Storage.h"
#include <ModbusRTUSlave.h>
// #include "src/Modbus/ModbusRTUSlave.h"

// ---------------------- Modbus Parameters ----------------------
extern int Baud_Rate;
extern int Slave_id;

// Function declarations
void modbus_init();
void modbus_loop();

#endif  // MODBUS_H

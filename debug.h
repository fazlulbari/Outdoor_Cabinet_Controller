#ifndef DEBUG_H
#define DEBUG_H

// ===== Global Debug Flags =====
extern bool DEBUG_MODE;
extern bool TASKDEBUG_MODE;
extern bool FANDEBUG_MODE;
extern bool SENSORSDEBUG_MODE;
extern bool MODBUSDEBUG_MODE;

// ===== Debug Serial Macros =====
#define DEBUG_BEGIN(baud)           do { Serial.begin(baud); while (!Serial); } while(0)

#define DEBUG_PRINT(x)              do { if (DEBUG_MODE) Serial.print(x); } while(0)
#define DEBUG_PRINTLN(x)            do { if (DEBUG_MODE) Serial.println(x); } while(0)

// ===== Task Debug Macros =====
#define TASK_DEBUG_PRINT(x)         do { if (TASKDEBUG_MODE) Serial.print(x); } while(0)
#define TASK_DEBUG_PRINTLN(x)       do { if (TASKDEBUG_MODE) Serial.println(x); } while(0)

// ===== Fan Debug Macros =====
#define FAN_DEBUG_PRINT(x)          do { if (FANDEBUG_MODE) Serial.print(x); } while(0)
#define FAN_DEBUG_PRINTLN(x)        do { if (FANDEBUG_MODE) Serial.println(x); } while(0)

// ===== Sensors Debug Macros =====
#define SENSORS_DEBUG_PRINT(x)      do { if (SENSORSDEBUG_MODE) Serial.print(x); } while(0)
#define SENSORS_DEBUG_PRINTLN(x)    do { if (SENSORSDEBUG_MODE) Serial.println(x); } while(0)

// ===== Modbus Debug Macros =====
#define MODBUS_DEBUG_PRINT(x)       do { if (MODBUSDEBUG_MODE) Serial.print(x); } while(0)
#define MODBUS_DEBUG_PRINTLN(x)     do { if (MODBUSDEBUG_MODE) Serial.println(x); } while(0)

#endif

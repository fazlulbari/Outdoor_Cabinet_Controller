#include "debug_utils.h"
#include "debug.h"  // for TASK_DEBUG_PRINT
#include <Arduino.h>
#include <STM32FreeRTOS.h>

void printTaskStackUsage(const char* taskName) {
  UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(NULL);
  TASK_DEBUG_PRINT("[");
  TASK_DEBUG_PRINT(taskName);
  TASK_DEBUG_PRINT("] Stack High Water Mark: ");
  TASK_DEBUG_PRINTLN(highWaterMark);
}

void handleSerialDebugCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "debug") {
      DEBUG_MODE = !DEBUG_MODE;
      Serial.println(DEBUG_MODE ? "Debugging enabled." : "Debugging disabled.");
    } else if (command == "taskdebug") {
      TASKDEBUG_MODE = !TASKDEBUG_MODE;
      Serial.println(TASKDEBUG_MODE ? "Task debugging enabled." : "Task debugging disabled.");
    } else if (command == "fandebug") {
      FANDEBUG_MODE = !FANDEBUG_MODE;
      Serial.println(FANDEBUG_MODE ? "Fan debugging enabled." : "Fan debugging disabled.");
    } else if (command == "sensorsdebug") {
      SENSORSDEBUG_MODE = !SENSORSDEBUG_MODE;
      Serial.println(SENSORSDEBUG_MODE ? "Sensors debugging enabled." : "Sensors debugging disabled.");
    } else if (command == "modbusdebug") {
      MODBUSDEBUG_MODE = !MODBUSDEBUG_MODE;
      Serial.println(MODBUSDEBUG_MODE ? "Modbus debugging enabled." : "Modbus debugging disabled.");
    }
  }
}

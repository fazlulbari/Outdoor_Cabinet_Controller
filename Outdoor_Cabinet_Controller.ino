#include "ODCC_config.h"
#include "menu.h"
#include "Fan_Controller.h"
#include "Sensors.h"
#include "Temperature_Sensor.h"
#include "Peripherals.h"
#include "modbus.h"
#include "Storage.h"
#include <STM32FreeRTOS.h>

FanController fans;
Sensors sensors;
TemperatureSensor temperatureSensor;
Peripherals peripherals;
Storage storage;


// start off with debug disabled
bool DEBUG_MODE = true;
bool TASKDEBUG_MODE = false;
bool FANDEBUG_MODE = false;
bool SENSORSDEBUG_MODE = false;
bool MODBUSDEBUG_MODE = false;


// Define timers for each alarm
static unsigned long doorCloseTimer = 0;
static unsigned long doorAlarmTimer = 0;
static unsigned long waterPresentTimer = 0;
static unsigned long smokeDetectedTimer = 0;
static unsigned long highTempTimer = 0;
static unsigned long fanFaultTimer = 0;


void LoopMenuTask(void *pvParameters) {
  static unsigned long lastTaskDebugPrint = 0;  // for Debugging Purpose
  // Infinite loop for menu tasks
  while (true) {
    loopMenu();

    // Check stack usage and print it out for debugging
    if (TASKDEBUG_MODE && millis() - lastTaskDebugPrint >= 3000) {
      lastTaskDebugPrint = millis();
      printTaskStackUsage("LoopMenuTask");
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);  // Add a small delay to prevent watchdog reset and allow other tasks to run
  }
}

void FansControlTask(void *pvParameters) {
  static unsigned long lastTaskDebugPrint = 0;  // for Debugging Purpose
  // Infinite loop for FansControl tasks
  while (true) {
    controlFansWithTemperature();

    // Check stack usage and print it out for debugging
    if (TASKDEBUG_MODE && millis() - lastTaskDebugPrint >= 3000) {
      lastTaskDebugPrint = millis();
      printTaskStackUsage("FansControlTask");
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);  // Add a small delay to prevent watchdog reset and allow other tasks to run
  }
}

void ModbusTask(void *pvParameters) {
  static unsigned long lastTaskDebugPrint = 0;  // for Debugging Purpose
  // Infinite loop for Modbus tasks
  while (true) {
    modbus_loop();

    // Check stack usage and print it out for debugging

    if (TASKDEBUG_MODE && millis() - lastTaskDebugPrint >= 3000) {
      lastTaskDebugPrint = millis();
      printTaskStackUsage("ModbusTask");
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);  // Add a small delay to prevent watchdog reset and allow other tasks to run
  }
}


void SensorsTask(void *pvParameters) {
  static unsigned long lastTaskDebugPrint = 0;  // for Debugging Purpose
  // Infinite loop for Sensors tasks
  while (true) {
    unsigned long now = millis();

    // Sensing Voltage
    inputVoltage = sensors.readInputVoltage(VOLTAGE_ADC_PIN, 100);
    SENSORS_DEBUG_PRINTLN();
    SENSORS_DEBUG_PRINTLN();
    SENSORS_DEBUG_PRINT("Input Voltage : ");
    SENSORS_DEBUG_PRINTLN(inputVoltage);

    // Sensing Temperature
    temperature = temperatureSensor.readTemperature();
    SENSORS_DEBUG_PRINT("Temperature   : ");
    SENSORS_DEBUG_PRINTLN(temperature);

    // Door Open Detection
    if (sensors.isDoorOpen()) {
      // Reset timers when door is open
      doorCloseTimer = 0;
      doorAlarmTimer = 0;

      // Immediately trigger light and alarm relay
      peripherals.controlLight(true);
      peripherals.triggerRelay(DOOR_ALARM, true);
      Door_open_alarm = true;
      SENSORS_DEBUG_PRINT("DOOR Opened |  ");
    } else {
      // Start light-off timer
      if (doorCloseTimer == 0) doorCloseTimer = now;
      if (now - doorCloseTimer >= ALARM_DELAY) {
        peripherals.controlLight(false);
      }

      // Start relay-off timer
      if (doorAlarmTimer == 0) doorAlarmTimer = now;
      if (now - doorAlarmTimer >= DOOR_ALARM_DELAY) {
        peripherals.triggerRelay(DOOR_ALARM, false);
        Door_open_alarm = false;
      }
      SENSORS_DEBUG_PRINT("DOOR Closed |  ");
    }

    // Water Presence Detection
    if (sensors.isWaterPresent()) {
      if (waterPresentTimer == 0) waterPresentTimer = now;
      if (now - waterPresentTimer >= ALARM_DELAY) {
        peripherals.triggerRelay(WATER_ALARM, true);
        Water_ingres_alarm = true;
        SENSORS_DEBUG_PRINT("WATER Ingressed |  ");
      }
    } else {
      waterPresentTimer = 0;
      peripherals.triggerRelay(WATER_ALARM, false);
      Water_ingres_alarm = false;
      SENSORS_DEBUG_PRINT("WATER Free      |  ");
    }

    // Smoke Detection
    if (sensors.isSmokeDetected()) {
      if (smokeDetectedTimer == 0) smokeDetectedTimer = now;
      if (now - smokeDetectedTimer >= ALARM_DELAY) {
        peripherals.triggerRelay(SMOKE_ALARM, true);
        Smoke_detected_alarm = true;
        SENSORS_DEBUG_PRINT("SMOKE Detected! |  ");
      }
    } else {
      smokeDetectedTimer = 0;
      peripherals.triggerRelay(SMOKE_ALARM, false);
      Smoke_detected_alarm = false;
      SENSORS_DEBUG_PRINT("SMOKE Free      |  ");
    }

    // High Temperature Detection
    if (temperature >= TEMP_HIGH_THRESHOLD) {
      if (highTempTimer == 0) highTempTimer = now;
      if (now - highTempTimer >= ALARM_DELAY) {
        peripherals.triggerRelay(HIGH_TEMP_ALARM, true);
        High_temperature_alarm = true;
        SENSORS_DEBUG_PRINT("TEMPERATURE High   |  ");
      }
    } else {
      highTempTimer = 0;
      peripherals.triggerRelay(HIGH_TEMP_ALARM, false);
      High_temperature_alarm = false;
      SENSORS_DEBUG_PRINT("TEMPERATURE Normal |  ");
    }

    // Fan Fault Detection
    if (fans.isFanFault()) {
      if (fanFaultTimer == 0) fanFaultTimer = now;
      if (now - fanFaultTimer >= FAN_FAULT_ALARM_DELAY) {
        peripherals.triggerRelay(FAN_FAULT_ALARM, true);
        Fan_fault_alarm = true;
      }
      SENSORS_DEBUG_PRINT("FAN Faulty");
    } else {
      fanFaultTimer = 0;
      peripherals.triggerRelay(FAN_FAULT_ALARM, false);
      Fan_fault_alarm = false;
      SENSORS_DEBUG_PRINT("FAN Ok");
    }

    // modbus_loop();


    // Handle Serial Debug Commands
    handleSerialDebugCommands();

    // Check stack usage and print it out for debugging
    if (TASKDEBUG_MODE && millis() - lastTaskDebugPrint >= 3000) {
      lastTaskDebugPrint = millis();
      printTaskStackUsage("SensorsTask");
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);  // Yield to other tasks
  }
}

void FansSubTask(void *pvParameters) {
  static unsigned long lastTaskDebugPrint = 0;  // for Debugging Purpose
  // Infinite loop for Fan tasks
  while (true) {

    static String lastFanType = "";
    static String lastFanProfile = "";

    if (FAN_TYPE != lastFanType || FAN_PROFILE != lastFanProfile) {
      fans.updateFanConfiguration();
      lastFanType = FAN_TYPE;
      lastFanProfile = FAN_PROFILE;
    }

    float inputVoltage = sensors.readInputVoltage(VOLTAGE_ADC_PIN, 100);

    if (inputVoltage < 45) {
      PWM_MAX_DUTY_FOR_VOLTAGE = 255;
    } else {
      PWM_MAX_DUTY_FOR_VOLTAGE = 255 - ((inputVoltage - 45) / (55 - 40)) * (255 - PWM_MAX_DUTY_FOR_FAN_TYPE);
    }
    // fans.updateFanStatus();

    // Check stack usage and print it out for debugging

    if (TASKDEBUG_MODE && millis() - lastTaskDebugPrint >= 3000) {
      lastTaskDebugPrint = millis();
      printTaskStackUsage("FansSubTask");
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);  // Add a delay to reduce CPU usage
  }
}

void controlFansWithTemperature() {
  static unsigned long lastToggleTime = 0;  // To keep track of the last toggle time
  static bool isFanPort1Active = true;      // To keep track of which fan is active
  static float currentSpeedFanPort1 = 0;    // Current speed of Fan 1
  static float currentSpeedFanPort2 = 0;    // Current speed of Fan 2
  float targetSpeed;

  FAN_DEBUG_PRINT("Fan_Model: ");
  FAN_DEBUG_PRINTLN(FAN_TYPE);
  FAN_DEBUG_PRINT("Max Speed: ");
  FAN_DEBUG_PRINTLN(PWM_MAX_DUTY_FOR_FAN_TYPE);
  FAN_DEBUG_PRINT("Min Current: ");
  FAN_DEBUG_PRINTLN(NOMINAL_CURRENT);
  FAN_DEBUG_PRINT("Max Curernt: ");
  FAN_DEBUG_PRINTLN(MAX_CURRENT);



  float currentTemperature = temperatureSensor.readTemperature();
  // float currentTemperature = 30;

  // FAN_DEBUG_PRINT("Current Temperature: ");
  // FAN_DEBUG_PRINT(currentTemperature);
  // FAN_DEBUG_PRINTLN(" °C");

  if (sensors.isSmokeDetected()) {
    FAN_DEBUG_PRINTLN("Smoke detected. Turning fans off.");
    fans.controlFan(1, 0);  // Turning fan off
    fans.controlFan(2, 0);  // Turning fan off
    currentSpeedFanPort1 = 0;
    currentSpeedFanPort2 = 0;
    return;
  } else if (currentTemperature < LOWER_TEMP_THRESHOLD) {
    FAN_DEBUG_PRINTLN("Temperature below First threshold. Turning fans off.");
    fans.controlFan(1, 0);  // Turning fan off
    fans.controlFan(2, 0);  // Turning fan off
    currentSpeedFanPort1 = 0;
    currentSpeedFanPort2 = 0;
  } else if (currentTemperature > UPPER_TEMP_THRESHOLD) {
    FAN_DEBUG_PRINTLN("Temperature above Second threshold. Running both fans.");
    // if Fan Profile TO-DO
    if (FAN_PROFILE == "AUTO") {

      FAN_DEBUG_PRINTLN("Fan_Profile = Auto PWM");
      FAN_DEBUG_PRINTLN();
      targetSpeed = ((currentTemperature - UPPER_TEMP_THRESHOLD) / (TEMP_HIGH_THRESHOLD - UPPER_TEMP_THRESHOLD) * 50) + 50;
      targetSpeed = constrain(targetSpeed, MIN_ACTIVE_SPEED, MAX_ACTIVE_SPEED);

      // Smoothly transition both fans to the target speed
      currentSpeedFanPort1 = smoothTransition(currentSpeedFanPort1, targetSpeed);
      currentSpeedFanPort2 = smoothTransition(currentSpeedFanPort2, targetSpeed);

      fans.controlFan(1, currentSpeedFanPort1);
      fans.controlFan(2, currentSpeedFanPort2);
    } else if (FAN_PROFILE == "NORMAL") {

      FAN_DEBUG_PRINTLN("Fan_Profile = NORMAL");
      FAN_DEBUG_PRINTLN();
      fans.controlFanSimple(1, 100);
      fans.controlFanSimple(2, 100);
    } else {
      //
    }
  } else {
    FAN_DEBUG_PRINTLN("Temperature between First and Second threshold. Running one fan and toggling.");
    targetSpeed = ((currentTemperature - LOWER_TEMP_THRESHOLD) / (UPPER_TEMP_THRESHOLD - LOWER_TEMP_THRESHOLD) * 50) + 50;
    targetSpeed = constrain(targetSpeed, MIN_ACTIVE_SPEED, MAX_ACTIVE_SPEED);

    FAN_DEBUG_PRINT(targetSpeed);
    FAN_DEBUG_PRINTLN(" %");
    FAN_DEBUG_PRINTLN("");

    // Check if it's time to toggle the active fan
    if (millis() - lastToggleTime >= TOGGLE_PERIOD * 60000) {
      isFanPort1Active = !isFanPort1Active;  // Toggle the active fan
      lastToggleTime = millis();             // Update the last toggle time
    }

    if (FAN_PROFILE == "AUTO") {  // Overlapping transition: Turn on the second fan before turning off the first

      FAN_DEBUG_PRINTLN("Fan_Profile = Auto PWM");
      FAN_DEBUG_PRINTLN();
      if (isFanPort1Active) {
        // Fan 1 is active, Fan 2 is turning on
        currentSpeedFanPort2 = smoothTransition(currentSpeedFanPort2, targetSpeed);  // Turn on Fan 2 first
        if (currentSpeedFanPort2 >= targetSpeed * 0.8) {                             // Wait until Fan 2 is mostly on
          currentSpeedFanPort1 = smoothTransition(currentSpeedFanPort1, 0);          // Then turn off Fan 1
        }
      } else {
        // Fan 2 is active, Fan 1 is turning on
        currentSpeedFanPort1 = smoothTransition(currentSpeedFanPort1, targetSpeed);  // Turn on Fan 1 first
        if (currentSpeedFanPort1 >= targetSpeed * 0.8) {                             // Wait until Fan 1 is mostly on
          currentSpeedFanPort2 = smoothTransition(currentSpeedFanPort2, 0);          // Then turn off Fan 2
        }
      }

      fans.controlFan(1, currentSpeedFanPort1);
      fans.controlFan(2, currentSpeedFanPort2);
    } else if (FAN_PROFILE == "NORMAL") {

      FAN_DEBUG_PRINTLN("Fan_Profile = NORMAL");
      FAN_DEBUG_PRINTLN();
      if (isFanPort1Active) {
        fans.controlFanSimple(2, 0);
        fans.controlFanSimple(1, 100);
      } else {
        fans.controlFanSimple(1, 0);
        fans.controlFanSimple(2, 100);
      }
    } else {
      //
    }
  }

  printTaskStackUsage("SensorsTask");

  vTaskDelay(500 / portTICK_PERIOD_MS);
}

// Helper function to smoothly transition between speeds
float smoothTransition(float currentSpeed, float targetSpeed) {
  const float transitionStep = 5.0;  // Adjust this value to control the smoothness of the transition
  if (currentSpeed < targetSpeed) {
    currentSpeed += transitionStep;
    if (currentSpeed > targetSpeed) {
      currentSpeed = targetSpeed;
    }
  } else if (currentSpeed > targetSpeed) {
    currentSpeed -= transitionStep;
    if (currentSpeed < targetSpeed) {
      currentSpeed = targetSpeed;
    }
  }
  return currentSpeed;
}

void setup() {
  Serial.begin(115200);  // Initialize Serial for debugging

  storage.begin();
  storage.loadFromEEPROM();

  analogReadResolution(12);

  setupMenu();
  fans.begin();
  sensors.begin();
  temperatureSensor.begin();
  peripherals.begin();
  modbus_init();
  delay(200);  // Initial delay for hardware to settle

  // peripherals.alertBuzzer(1);
  delay(200);

  DEBUG_PRINT("Free heap before task creation: ");
  DEBUG_PRINTLN(xPortGetFreeHeapSize());

  // Create FreeRTOS tasks

  // if (xTaskCreate(FansControlTask, "Fan Control Task", 128 * 1, NULL, 2, NULL) != pdPASS) {
  //   DEBUG_PRINTLN("Fan Task creation failed!");
  // }

  if (xTaskCreate(SensorsTask, "Sensor Task", 256 * 1, NULL, 1, NULL) != pdPASS) {
    DEBUG_PRINTLN("Sensor Task creation failed!");
  }

  if (xTaskCreate(FansSubTask, "Fan Sub Task", 256 * 1, NULL, 2, NULL) != pdPASS) {
    DEBUG_PRINTLN("Fan Sub Task creation failed!");
  }

  if (xTaskCreate(LoopMenuTask, "Loop Menu Task", 256 * 1, NULL, 1, NULL) != pdPASS) {
    DEBUG_PRINTLN("Menu Task creation failed!");
  }

  if (xTaskCreate(ModbusTask, "Modbus Task", 256 * 1, NULL, 2, NULL) != pdPASS) {
    DEBUG_PRINTLN("Modbus Task creation failed!");
  }

  DEBUG_PRINT("Free heap after task creation: ");
  DEBUG_PRINTLN(xPortGetFreeHeapSize());



  // Start the scheduler
  vTaskStartScheduler();
}

void loop() {
  static unsigned long lastTaskDebugPrint = 0;  // for Debugging Purpose

  controlFansWithTemperature();

  // Print stack usage for loop task
  if (TASKDEBUG_MODE && millis() - lastTaskDebugPrint >= 3000) {
    lastTaskDebugPrint = millis();
    printTaskStackUsage("MainLoop");
  }

  vTaskDelay(100 / portTICK_PERIOD_MS);
}

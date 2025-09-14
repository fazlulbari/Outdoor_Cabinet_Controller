#include "Fan_Controller.h"


extern Sensors sensors;

// Constants
const float SHUNT_RESISTOR = 1.0;
float SCALING_FACTOR = (ADC_MAX_VOLTAGE / STM32_ADC_RESOLUTION) / SHUNT_RESISTOR * 1000;  // Scaling factor in mA per ADC unit:

uint8_t VOLTAGE_SAMPLE_COUNT = 100;
int CURRENT_SAMPLE_COUNT = 100;

float NOMINAL_CURRENT = 150;  // Nominal current per fan 150 mA
float MAX_CURRENT = 400;      // max current per fan 200 mA
float MAX_POWER = 9;          // max current per fan 7W
int NUM_OF_FAN_PER_PORT = 2;  // Number of Fan Connected to each port

float maxAllowedCurrent;
float maxAllowedPower;

unsigned long fanPort1StartTime = 0;
unsigned long fanPort2StartTime = 0;

const unsigned long IN_RUSH_IGNORE_TIME = 5000;  // 5 seconds


bool skipFaultCheck1 = false;
bool skipFaultCheck2 = false;

int PWM_MAX_DUTY_FOR_FAN_TYPE = 150;  // initilize to 200 max 255

int PWM_MAX_DUTY_FOR_VOLTAGE = 220;  // initilize to 200 max 255
int PWM_MAX_DUTY_FOR_CURRENT = 200;  // initilize to 200 max 255

int PWM_MAX_DUTY = 255;                    // initilize to 200 max 255
float lastStableDutyCycle = PWM_MAX_DUTY;  // Store last safe duty cycle

int reductionStep = 1;

int PWM_MIN_DUTY = 0;
int PWM_MIN_ACTIVE_DUTY = 100;

int MIN_ACTIVE_SPEED = 70;
int MAX_ACTIVE_SPEED = 100;



// Fan status, current, and power variables
bool Fan_Current_Unit = false;  // Default to mAmp

bool FanPort1_status = false;  // Initialize Fan 1 status to idle
float FanPort1_current = 0.0;  // Initialize Fan 1 current to 0 A
float FanPort1_power = 0.0;    // Initialize Fan 1 power to 0 W

bool FanPort2_status = false;  // Initialize Fan 2 status to idle
float FanPort2_current = 0.0;  // Initialize Fan 2 current to 0 A
float FanPort2_power = 0.0;    // Initialize Fan 2 power to 0 W

// bool Fan_fault = false;                // Global fault flag

// Individual fan fault flags
bool FanPort1_fault = false;  // Fan 1 fault flag
bool FanPort2_fault = false;  // Fan 2 fault flag


FaultType FanPort1_fault_Status = NO_FAULT;
FaultType FanPort2_fault_Status = NO_FAULT;

FaultType lastFanPort1Fault = NO_FAULT;
FaultType lastFanPort2Fault = NO_FAULT;



// ---------------- Temperature Threshold Values -----------------
int LOWER_TEMP_THRESHOLD = 27;     // First  temperature threshold (in °C)
int UPPER_TEMP_THRESHOLD = 30;     // Second temperature threshold (in °C)
int TEMP_HIGH_THRESHOLD  = 32;     // Third  temperature threshold (in °C)
int TOGGLE_PERIOD        = 30;     // Toggle period (in Minutes)

// -------------------- Fan Configuration ------------------------
String FAN_TYPE = ""; // To be set dynamically

String FAN_MODEL[] = {"KRUBO", "DELTA", "CUSTOM"};
uint8_t NUM_FAN_MODEL = sizeof(FAN_MODEL) / sizeof(FAN_MODEL[0]);

String FAN_PROFILE = ""; // To be set dynamically

String FAN_MODE[] = {"NORMAL", "AUTO"};
uint8_t NUM_FAN_MODE = sizeof(FAN_MODE) / sizeof(FAN_MODE[0]);

int CUSTOM_MAX_FAN_SPEED = 230; // PWM max or RPM max depending on context

// --------------- Fan Current Thresholds (mA) -------------------
int DOUBLE_FAN_OPEN_CURRENT_THRESHOLD  = 100;
int SINGLE_FAN_OPEN_CURRENT_THRESHOLD  = 200;

int SINGLE_FAN_JAM_CURRENT_THRESHOLD   = 320;
int DOUBLE_FAN_JAM_CURRENT_THRESHOLD   = 370;

int SINGLE_FAN_SHORT_CURRENT_THRESHOLD = 420;
int DOUBLE_FAN_SHORT_CURRENT_THRESHOLD = 470;


// Fault timers for each fan
unsigned long faultStartTimeFanPort1 = 0;   // Time when Fan 1 fault was detected
unsigned long faultStartTimeFanPort2 = 0;   // Time when Fan 2 fault was detected
uint32_t faultStartTime = 0;                // Time when the fault was detected
uint32_t FAULT_RECOVERY_DELAY = 30 * 1000;  // Fault Recovery Delay (30 seconds)
uint32_t NO_FAN_THRESHOLD_TIME = 3 * 1000;  // 3 seconds threshold


FanController::FanController() {}

FanConfig activeFanConfig;

void FanController::begin() {
  setupPins();
  activeFanConfig = getFanConfig(FAN_TYPE, FAN_PROFILE);

  PWM_MAX_DUTY_FOR_FAN_TYPE = activeFanConfig.pwmMaxDuty;
  NOMINAL_CURRENT = activeFanConfig.nominalCurrent;
  MAX_CURRENT = activeFanConfig.maxCurrent;

  DOUBLE_FAN_OPEN_CURRENT_THRESHOLD = activeFanConfig.doubleFanOpenThreshold;
  SINGLE_FAN_OPEN_CURRENT_THRESHOLD = activeFanConfig.singleFanOpenThreshold;
  SINGLE_FAN_JAM_CURRENT_THRESHOLD = activeFanConfig.singleFanJamThreshold;
  DOUBLE_FAN_JAM_CURRENT_THRESHOLD = activeFanConfig.doubleFanJamThreshold;
  SINGLE_FAN_SHORT_CURRENT_THRESHOLD = activeFanConfig.singleFanShortThreshold;
  DOUBLE_FAN_SHORT_CURRENT_THRESHOLD = activeFanConfig.doubleFanShortThreshold;

  maxAllowedCurrent = MAX_CURRENT;
  maxAllowedPower = inputVoltage * maxAllowedCurrent * 0.001f;
}


void FanController::setupPins() {
  pinMode(FANPORT1_PWM_PIN, OUTPUT);
  pinMode(FANPORT1_SHUNT_PIN, INPUT);
  pinMode(FANPORT2_PWM_PIN, OUTPUT);
  pinMode(FANPORT2_SHUNT_PIN, INPUT);
}

void FanController::controlFan(int fanPort, float desiredSpeed) {
  int pwmPin = (fanPort == 1) ? FANPORT1_PWM_PIN : FANPORT2_PWM_PIN;
  int shuntPin = (fanPort == 1) ? FANPORT1_SHUNT_PIN : FANPORT2_SHUNT_PIN;
  lastStableDutyCycle = PWM_MAX_DUTY;
  int dutyCycle = 0;

  // Read input voltage and fan current
  inputVoltage = sensors.readInputVoltage(VOLTAGE_ADC_PIN, VOLTAGE_SAMPLE_COUNT);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  float fanCurrent = readFanCurrent(shuntPin, CURRENT_SAMPLE_COUNT);
  // float fanCurrent = 300;

  float fanPower = inputVoltage * fanCurrent * 0.001f;

  // Update fan status, current, and power
  updateFanStatus(fanPort, desiredSpeed, fanCurrent, fanPower);

  // Check if the fan is in fault recovery mode
  if (fanPort == 1 && FanPort1_fault) {
    if (millis() - faultStartTimeFanPort1 >= FAULT_RECOVERY_DELAY) {
      // Fault recovery delay has passed, reset the fault flag
      FanPort1_fault = false;
      FAN_DEBUG_PRINTLN("Fan 1 fault recovery complete. Resetting fan.");
    } else {
      // Fan 1 is still in fault recovery mode, keep it off
      // turnOffFan(fanPort, pwmPin);
      return;
    }
  } else if (fanPort == 2 && FanPort2_fault) {
    if (millis() - faultStartTimeFanPort2 >= FAULT_RECOVERY_DELAY) {
      // Fault recovery delay has passed, reset the fault flag
      FanPort2_fault = false;
      FAN_DEBUG_PRINTLN("Fan 2 fault recovery complete. Resetting fan.");
    } else {
      // Fan 2 is still in fault recovery mode, keep it off
      // turnOffFan(fanPort, pwmPin);
      return;
    }
  }

  // Turn off fan if desired speed is 0 or below threshold
  if (desiredSpeed <= MIN_ACTIVE_SPEED) {
    turnOffFan(fanPort, pwmPin);
    return;
  }

  // Constrain desired speed to 100%
  if (desiredSpeed > 100) desiredSpeed = 100;


  // Check for safety conditions (over-current, over-voltage, etc.)
  if (checkSafetyConditions(fanPort, pwmPin, inputVoltage, fanCurrent, fanPower)) {
    return;
  }


  // Adjust duty cycle based on power limits
  dutyCycle = adjustDutyCycle(fanPort, desiredSpeed, fanPower, inputVoltage, fanCurrent);

  // Apply the duty cycle to the fan
  analogWrite(pwmPin, dutyCycle);

  // Log the results
  logFanStatus(fanPort, inputVoltage, fanCurrent, fanPower, dutyCycle);

  // Detect and handle faults
  FaultType fault = detectFault(fanPort, fanCurrent);
  // if (fault != NO_FAULT) {
  //   handleFault(fanPort, fault);
  //   return;
  // }
}


void FanController::controlFanSimple(int fanPort, float desiredSpeed) {
  int pwmPin = (fanPort == 1) ? FANPORT1_PWM_PIN : FANPORT2_PWM_PIN;
  int shuntPin = (fanPort == 1) ? FANPORT1_SHUNT_PIN : FANPORT2_SHUNT_PIN;

  inputVoltage = sensors.readInputVoltage(VOLTAGE_ADC_PIN, VOLTAGE_SAMPLE_COUNT);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  float fanCurrent = readFanCurrent(shuntPin, CURRENT_SAMPLE_COUNT);
  float fanPower = inputVoltage * fanCurrent * 0.001f;

  // Update fan measurements (but not status flag here)
  updateFanStatus(fanPort, desiredSpeed, fanCurrent, fanPower);

  // Fault recovery check
  if ((fanPort == 1 && FanPort1_fault && millis() - faultStartTimeFanPort1 < FAULT_RECOVERY_DELAY) || (fanPort == 2 && FanPort2_fault && millis() - faultStartTimeFanPort2 < FAULT_RECOVERY_DELAY)) {
    return;
  }

  // Reset fault after recovery delay
  if (fanPort == 1 && FanPort1_fault) {
    FanPort1_fault = false;
    FAN_DEBUG_PRINTLN("Fan 1 fault recovery complete. Resetting fan.");
  }
  if (fanPort == 2 && FanPort2_fault) {
    FanPort2_fault = false;
    FAN_DEBUG_PRINTLN("Fan 2 fault recovery complete. Resetting fan.");
  }

  if (checkSafetyConditions(fanPort, pwmPin, inputVoltage, fanCurrent, fanPower)) {
    return;
  }

  // Turn off fan if below threshold
  if (desiredSpeed <= MIN_ACTIVE_SPEED) {
    turnOffFan(fanPort, pwmPin);
    if (fanPort == 1) FanPort1_status = false;
    if (fanPort == 2) FanPort2_status = false;
    return;
  }

  // Only set start time if turning on for the first time
  if (fanPort == 1 && !FanPort1_status) {
    fanPort1StartTime = millis();
  }
  if (fanPort == 2 && !FanPort2_status) {
    fanPort2StartTime = millis();
  }

  analogWrite(pwmPin, PWM_MAX_DUTY_FOR_VOLTAGE);

  // Fan is now ON
  if (fanPort == 1) FanPort1_status = true;
  if (fanPort == 2) FanPort2_status = true;

  // Fault detection
  unsigned long now = millis();
  if (fanPort == 1) {
    skipFaultCheck1 = (now - fanPort1StartTime <= IN_RUSH_IGNORE_TIME);
    if (!skipFaultCheck1) {
      FaultType fault = detectFault(fanPort, fanCurrent);
      handleFault(fanPort, fault);
    } else {
      FanPort1_fault = false;
      FanPort1_fault_Status = NO_FAULT;
      FAN_DEBUG_PRINTLN("Fan 1: Inrush ignore - skipping fault detection.");
    }
  } else if (fanPort == 2) {
    skipFaultCheck2 = (now - fanPort2StartTime <= IN_RUSH_IGNORE_TIME);
    if (!skipFaultCheck2) {
      FaultType fault = detectFault(fanPort, fanCurrent);
      handleFault(fanPort, fault);
    } else {
      FanPort2_fault = false;
      FanPort2_fault_Status = NO_FAULT;
      FAN_DEBUG_PRINTLN("Fan 2: Inrush ignore - skipping fault detection.");
    }
  }
}



FaultType FanController::detectFault(int fanPort, float fanCurrent) {
  FAN_DEBUG_PRINT("FAN Port : ");
  FAN_DEBUG_PRINT(fanPort);
  FAN_DEBUG_PRINT(" Current ");
  FAN_DEBUG_PRINTLN(fanCurrent);
  // Check for short circuit conditions first (most severe)
  if (fanCurrent >= DOUBLE_FAN_SHORT_CURRENT_THRESHOLD) {
    if (fanPort == 1) {
      FanPort1_fault = true;
      FanPort1_fault_Status = FAN_SHORT_2;  // Double fan short
      faultStartTimeFanPort1 = millis();
      FAN_DEBUG_PRINTLN("Fan 1 Double Fan Short Circuit Detected");
    } else if (fanPort == 2) {
      FanPort2_fault = true;
      FanPort2_fault_Status = FAN_SHORT_2;  // Double fan short
      faultStartTimeFanPort2 = millis();
      FAN_DEBUG_PRINTLN("Fan 2 Double Fan Short Circuit Detected");
    }
    return FAN_SHORT_2;
  }
  // add Comment
  else if (fanCurrent >= SINGLE_FAN_SHORT_CURRENT_THRESHOLD) {
    if (fanPort == 1) {
      FanPort1_fault = true;
      FanPort1_fault_Status = FAN_SHORT_1;  // Single fan short
      faultStartTimeFanPort1 = millis();
      FAN_DEBUG_PRINTLN("Fan 1 Single Fan Short Circuit Detected");
    } else if (fanPort == 2) {
      FanPort2_fault = true;
      FanPort2_fault_Status = FAN_SHORT_1;  // Single fan short
      faultStartTimeFanPort2 = millis();
      FAN_DEBUG_PRINTLN("Fan 2 Single Fan Short Circuit Detected");
    }
    return FAN_SHORT_1;
  }
  // Check for jam conditions
  else if (fanCurrent >= DOUBLE_FAN_JAM_CURRENT_THRESHOLD) {
    if (fanPort == 1) {
      FanPort1_fault = true;
      FanPort1_fault_Status = FAN_JAM_2;  // Double fan jam
      faultStartTimeFanPort1 = millis();
      FAN_DEBUG_PRINTLN("Fan 1 Double Fan Jam Detected");
    } else if (fanPort == 2) {
      FanPort2_fault = true;
      FanPort2_fault_Status = FAN_JAM_2;  // Double fan jam
      faultStartTimeFanPort2 = millis();
      FAN_DEBUG_PRINTLN("Fan 2 Double Fan Jam Detected");
    }
    return FAN_JAM_2;
  }
  // add Comment
  else if (fanCurrent >= SINGLE_FAN_JAM_CURRENT_THRESHOLD) {
    if (fanPort == 1) {
      FanPort1_fault = true;
      FanPort1_fault_Status = FAN_JAM_1;  // Single fan jam
      faultStartTimeFanPort1 = millis();
      FAN_DEBUG_PRINTLN("Fan 1 Single Fan Jam Detected");
    } else if (fanPort == 2) {
      FanPort2_fault = true;
      FanPort2_fault_Status = FAN_JAM_1;  // Single fan jam
      faultStartTimeFanPort2 = millis();
      FAN_DEBUG_PRINTLN("Fan 2 Single Fan Jam Detected");
    }
    return FAN_JAM_1;
  }

  // Check for no fan condition
  else if (fanCurrent <= 10) {
    unsigned long currentTime = millis();
    if (fanPort == 1) {
      if (faultStartTimeFanPort1 == 0) {
        faultStartTimeFanPort1 = currentTime;  // Start the timer
      } else if (currentTime - faultStartTimeFanPort1 > NO_FAN_THRESHOLD_TIME) {
        FanPort1_fault = true;
        FanPort1_fault_Status = NO_FAN;
        FAN_DEBUG_PRINTLN("Fan 1 NOFAN");
      }
    } else if (fanPort == 2) {
      if (faultStartTimeFanPort2 == 0) {
        faultStartTimeFanPort2 = currentTime;  // Start the timer
      } else if (currentTime - faultStartTimeFanPort2 > NO_FAN_THRESHOLD_TIME) {
        FanPort2_fault = true;
        FanPort2_fault_Status = NO_FAN;
        FAN_DEBUG_PRINTLN("Fan 2 NOFAN");
      }
    }
    return NO_FAN;
  }
  // Check for fan open condition
  else if (fanCurrent <= SINGLE_FAN_OPEN_CURRENT_THRESHOLD) {
    if (fanPort == 1) {
      FanPort1_fault = true;
      FanPort1_fault_Status = FAN_OPEN_1;
      faultStartTimeFanPort1 = millis();
      FAN_DEBUG_PRINTLN("Fan 1 Open Detected (Single Fan Open)");
    } else if (fanPort == 2) {
      FanPort2_fault = true;
      FanPort2_fault_Status = FAN_OPEN_1;
      faultStartTimeFanPort2 = millis();
      FAN_DEBUG_PRINTLN("Fan 2 Open Detected (Single Fan Open)");
    }
    return FAN_OPEN_1;
  }
  // add Comment
  else if (fanCurrent <= DOUBLE_FAN_OPEN_CURRENT_THRESHOLD) {
    if (fanPort == 1) {
      FanPort1_fault = true;
      FanPort1_fault_Status = FAN_OPEN_2;
      faultStartTimeFanPort1 = millis();
      FAN_DEBUG_PRINTLN("Fan 1 Open Detected (Both Fans Open)");
    } else if (fanPort == 2) {
      FanPort2_fault = true;
      FanPort2_fault_Status = FAN_OPEN_2;
      faultStartTimeFanPort2 = millis();
      FAN_DEBUG_PRINTLN("Fan 2 Open Detected (Both Fans Open)");
    }
    return FAN_OPEN_2;
  }
  // add Comment  
  else {
    // No fault condition
    if (fanPort == 1) {
      faultStartTimeFanPort1 = 0;
      FanPort1_fault = false;
      FanPort1_fault_Status = NO_FAULT;
      FAN_DEBUG_PRINTLN("Fan 1 NO Fault");
    }
    if (fanPort == 2) {
      faultStartTimeFanPort2 = 0;
      FanPort2_fault = false;
      FanPort2_fault_Status = NO_FAULT;
      FAN_DEBUG_PRINTLN("Fan 2 NO Fault");
    }
  }
  return NO_FAULT;
}

bool FanController::isFanFault() {
  return FanPort1_fault || FanPort2_fault;
}

void FanController::handleFault(int fanPort, FaultType fault) {
  int pwmPin = (fanPort == 1) ? FANPORT1_PWM_PIN : FANPORT2_PWM_PIN;

  switch (fault) {
    case FAN_JAM:
    case FAN_JAM_1:
    case FAN_JAM_2:
      // turnOffFan(fanPort, pwmPin);
      FAN_DEBUG_PRINT("Fan ");
      FAN_DEBUG_PRINT(fanPort);
      FAN_DEBUG_PRINTLN(" Jam Detected! Turning off fan.");
      break;

    case FAN_SHORT:
    case FAN_SHORT_1:
    case FAN_SHORT_2:
      // turnOffFan(fanPort, pwmPin);
      FAN_DEBUG_PRINT("Fan ");
      FAN_DEBUG_PRINT(fanPort);
      FAN_DEBUG_PRINTLN(" Short Circuit Detected! Turning off fan.");
      break;

    case NO_FAN:
      // turnOffFan(fanPort, pwmPin);
      FAN_DEBUG_PRINT("Fan ");
      FAN_DEBUG_PRINT(fanPort);
      FAN_DEBUG_PRINTLN(" Not Detected! Turning off fan.");
      break;
    case FAN_OPEN_1:
    case FAN_OPEN_2:
      // turnOffFan(fanPort, pwmPin);
      FAN_DEBUG_PRINT("Fan ");
      FAN_DEBUG_PRINT(fanPort);
      FAN_DEBUG_PRINTLN(" Open Detected! Turning off fan.");
      break;

    default:
      FAN_DEBUG_PRINT("Fan ");
      FAN_DEBUG_PRINT(fanPort);
      FAN_DEBUG_PRINTLN(" Unknown Fault Type.");
      break;
  }
}

void FanController::turnOffFan(int fanPort, int pwmPin) {
  analogWrite(pwmPin, 0);

  if (fanPort == 1) {
    FanPort1_status = false;
    FanPort1_current = 0.0;
    FanPort1_power = 0.0;
  } else if (fanPort == 2) {
    FanPort2_status = false;
    FanPort2_current = 0.0;
    FanPort2_power = 0.0;
  }
}

int FanController::adjustDutyCycle(int fanPort, float desiredSpeed, float fanPower, float inputVoltage, float fanCurrent) {

  int pwmPin = (fanPort == 1) ? FANPORT1_PWM_PIN : FANPORT2_PWM_PIN;
  int shuntPin = (fanPort == 1) ? FANPORT1_SHUNT_PIN : FANPORT2_SHUNT_PIN;


  int dutyCycle = map(desiredSpeed, MIN_ACTIVE_SPEED, MAX_ACTIVE_SPEED, PWM_MIN_ACTIVE_DUTY, PWM_MAX_DUTY_FOR_VOLTAGE);
  dutyCycle = constrain(dutyCycle, PWM_MIN_DUTY, PWM_MAX_DUTY_FOR_VOLTAGE);

  if (fanPower > maxAllowedPower) {
    while (fanPower > maxAllowedPower && dutyCycle > PWM_MIN_DUTY) {
      dutyCycle -= reductionStep;
      dutyCycle = max(dutyCycle, PWM_MIN_DUTY);
      analogWrite(pwmPin, dutyCycle);
      fanPower = inputVoltage * readFanCurrent(shuntPin, CURRENT_SAMPLE_COUNT) * 0.001f;
    }
  }
  return dutyCycle;
}

void FanController::updateFanStatus(int fanPort, float desiredSpeed, float fanCurrent, float fanPower) {
  int shuntPin = (fanPort == 1) ? FANPORT1_SHUNT_PIN : FANPORT2_SHUNT_PIN;

  inputVoltage = sensors.readInputVoltage(VOLTAGE_ADC_PIN, VOLTAGE_SAMPLE_COUNT);

  if (fanPort == 1) {
    FanPort1_current = readFanCurrent(shuntPin, CURRENT_SAMPLE_COUNT);
    FanPort1_power = inputVoltage * FanPort1_current * 0.001f;
  } else if (fanPort == 2) {
    FanPort2_current = readFanCurrent(shuntPin, CURRENT_SAMPLE_COUNT);
    FanPort2_power = inputVoltage * FanPort2_current * 0.001f;
  }
}

void FanController::logFanStatus(int fanPort, float inputVoltage, float fanCurrent, float fanPower, int dutyCycle) {
  FAN_DEBUG_PRINT("Fan ");
  FAN_DEBUG_PRINT(fanPort);
  FAN_DEBUG_PRINT(" | Input Voltage: ");
  FAN_DEBUG_PRINT(inputVoltage);
  FAN_DEBUG_PRINT(" V | Fan Current: ");
  FAN_DEBUG_PRINT(fanCurrent);
  FAN_DEBUG_PRINT(" A | Fan Power: ");
  FAN_DEBUG_PRINT(fanPower);
  FAN_DEBUG_PRINT(" W | PWM Duty Cycle: ");
  FAN_DEBUG_PRINT(dutyCycle);
  FAN_DEBUG_PRINTLN("/255");
}

bool FanController::checkSafetyConditions(int fanPort, int pwmPin, float inputVoltage, float fanCurrent, float fanPower) {

  if (fanCurrent >= maxAllowedCurrent || inputVoltage <= VOLTAGE_LOW_THRESHOLD || inputVoltage >= VOLTAGE_HIGH_THRESHOLD) {
    turnOffFan(fanPort, pwmPin);
    return true;
  }
  return false;
}



FanConfig FanController::getFanConfig(String fanType, String fanProfile) {
  if (fanType == "KRUBO") {
    return FanConfig{
      .pwmMaxDuty = 200,
      .nominalCurrent = 150,
      .maxCurrent = 220,
      .doubleFanOpenThreshold = 100,
      .singleFanOpenThreshold = 200,
      .singleFanJamThreshold = 320,
      .doubleFanJamThreshold = 370,
      .singleFanShortThreshold = 420,
      .doubleFanShortThreshold = 470
    };
  } else if (fanType == "DELTA") {
    return FanConfig{
      .pwmMaxDuty = 120,
      .nominalCurrent = 150,
      .maxCurrent = 250,
      .doubleFanOpenThreshold = 120,
      .singleFanOpenThreshold = 220,
      .singleFanJamThreshold = 320,
      .doubleFanJamThreshold = 390,
      .singleFanShortThreshold = 440,
      .doubleFanShortThreshold = 490
    };
  } else if (fanType == "CUSTOM") {
    // Allow user to define from EEPROM or settings menu
    return FanConfig{
      .pwmMaxDuty = CUSTOM_MAX_FAN_SPEED,
      .nominalCurrent = NOMINAL_CURRENT,
      .maxCurrent = MAX_CURRENT,
      .doubleFanOpenThreshold = DOUBLE_FAN_OPEN_CURRENT_THRESHOLD,
      .singleFanOpenThreshold = SINGLE_FAN_OPEN_CURRENT_THRESHOLD,
      .singleFanJamThreshold = SINGLE_FAN_JAM_CURRENT_THRESHOLD,
      .doubleFanJamThreshold = DOUBLE_FAN_JAM_CURRENT_THRESHOLD,
      .singleFanShortThreshold = SINGLE_FAN_SHORT_CURRENT_THRESHOLD,
      .doubleFanShortThreshold = DOUBLE_FAN_SHORT_CURRENT_THRESHOLD
    };
  }
  // Default fallback
  return FanConfig{};
}

void FanController::updateFanConfiguration() {
  FanConfig config = getFanConfig(FAN_TYPE, FAN_PROFILE);

  PWM_MAX_DUTY_FOR_FAN_TYPE = config.pwmMaxDuty;
  NOMINAL_CURRENT = config.nominalCurrent;
  MAX_CURRENT = config.maxCurrent;

  SINGLE_FAN_OPEN_CURRENT_THRESHOLD = config.singleFanOpenThreshold;
  DOUBLE_FAN_OPEN_CURRENT_THRESHOLD = config.doubleFanOpenThreshold;
  SINGLE_FAN_JAM_CURRENT_THRESHOLD = config.singleFanJamThreshold;
  DOUBLE_FAN_JAM_CURRENT_THRESHOLD = config.doubleFanJamThreshold;
  SINGLE_FAN_SHORT_CURRENT_THRESHOLD = config.singleFanShortThreshold;
  DOUBLE_FAN_SHORT_CURRENT_THRESHOLD = config.doubleFanShortThreshold;

  maxAllowedCurrent = MAX_CURRENT;
  maxAllowedPower = inputVoltage * maxAllowedCurrent * 0.001f;
}


float FanController::readFanCurrent(int shuntPin, int sampleCount) {
  float total = 0.0;
  for (int i = 0; i < sampleCount; i++) {
    analogReadResolution(12);
    total += analogRead(shuntPin);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  float averageAdcValue = total / sampleCount;
  return averageAdcValue * SCALING_FACTOR;
}

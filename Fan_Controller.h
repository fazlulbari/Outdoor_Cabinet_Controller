#ifndef FAN_CONTROLLER_H
#define FAN_CONTROLLER_H

#include "Arduino.h"
#include "ODCC_config.h"
#include "Sensors.h"
#include <STM32FreeRTOS.h>
// =============================================
// Constants and Configuration
// =============================================

// Electrical Constants
extern const float SHUNT_RESISTOR;  // Shunt resistor value in ohms
extern float SCALING_FACTOR;        // Shunt resistor value in ohms

extern uint8_t VOLTAGE_SAMPLE_COUNT;
extern int CURRENT_SAMPLE_COUNT;

extern float NOMINAL_CURRENT;  // Nominal current per fan in mA
extern float MAX_CURRENT;      // Maximum allowed current per fan (in Amperes)
extern float MAX_POWER;        // Maximum allowed power per fan (in Watts)

extern int NUM_OF_FAN_PER_PORT;  // Number of fans connected

// Derived Limits
extern float maxAllowedCurrent;  // Maximum allowed current for all fans combined
extern float maxAllowedPower;    // Maximum allowed power for all fans combined

// PWM Configuration
extern int PWM_MAX_DUTY_FOR_FAN_TYPE;  // Maximum duty cycle based on FAN_TYPE (0-255)

extern int PWM_MAX_DUTY_FOR_VOLTAGE;  // Maximum duty cycle based on voltage (0-255)
extern int PWM_MAX_DUTY_FOR_CURRENT;  // Maximum duty cycle based on current (0-255)
extern int PWM_MAX_DUTY;              // Overall maximum duty cycle (0-255)
extern float lastStableDutyCycle;     // Last stable duty cycle used for safety

extern int PWM_MIN_DUTY;         // Minimum duty cycle (0-255)
extern int PWM_MIN_ACTIVE_DUTY;  // Minimum duty cycle for active fan operation (0-255)

// Fan Speed Configuration
extern int MIN_ACTIVE_SPEED;  // Minimum active speed (in percentage)
extern int MAX_ACTIVE_SPEED;  // Maximum active speed (in percentage)

// =============================================
// Fan Status and Metrics
// =============================================

// Fan 1 Metrics
extern bool FanPort1_status;    // Fan 1 status (true = running, false = idle)
extern float FanPort1_current;  // Fan 1 current in Amperes
extern float FanPort1_power;    // Fan 1 power in Watts

// Fan 2 Metrics
extern bool FanPort2_status;    // Fan 2 status (true = running, false = idle)
extern float FanPort2_current;  // Fan 2 current in Amperes
extern float FanPort2_power;    // Fan 2 power in Watts

extern bool Fan_fault;  // Fan Fault

extern bool FanPort1_fault;  // Fan 1 Fault
extern bool FanPort2_fault;  // Fan 2 Fault

extern uint32_t faultStartTime;         // Time when the fault was detected
extern uint32_t FAULT_RECOVERY_DELAY;   // Fault Recovery Delay (5 seconds)
extern uint32_t NO_FAN_THRESHOLD_TIME;  // 3 seconds threshold

// Unit Configuration
extern bool Fan_Current_Unit;  // Current unit (true = Amperes, false = Milliamperes)




struct FanConfig {
  int pwmMaxDuty;
  float nominalCurrent;
  float maxCurrent;
  int doubleFanOpenThreshold;
  int singleFanOpenThreshold;
  int singleFanJamThreshold;
  int doubleFanJamThreshold;
  int singleFanShortThreshold;
  int doubleFanShortThreshold;
};


// Fault Types
enum FaultType {
  NO_FAULT,
  NO_FAN,
  FAN_OPEN_1,
  FAN_OPEN_2,
  FAN_JAM,
  FAN_JAM_1,
  FAN_JAM_2,
  FAN_SHORT,
  FAN_SHORT_1,
  FAN_SHORT_2
};

// FanState Struct for each port
struct FanState {
  bool status = false;
  bool fault = false;
  float current = 0.0;
  float power = 0.0;
  FaultType faultStatus = NO_FAULT;
  unsigned long faultStartTime = 0;
  unsigned long onStartTime = 0;
};



// Externally accessible fan port array
extern FanState fanPorts[2];




extern FaultType FanPort1_fault_Status;
extern FaultType FanPort2_fault_Status;

extern FaultType lastFanPort1Fault;
extern FaultType lastFanPort2Fault;

// Constants for Fault Detection

// ---------------- Temperature Threshold Values -----------------
extern int LOWER_TEMP_THRESHOLD;
extern int UPPER_TEMP_THRESHOLD;
extern int TEMP_HIGH_THRESHOLD;
extern int TOGGLE_PERIOD;

// -------------------- Fan Configuration ------------------------
extern String FAN_TYPE;
extern String FAN_MODEL[];
extern uint8_t NUM_FAN_MODEL;

extern String FAN_PROFILE;
extern String FAN_MODE[];
extern uint8_t NUM_FAN_MODE;

extern int CUSTOM_MAX_FAN_SPEED;

// --------------- Fan Current Thresholds (mA) -------------------
extern int SINGLE_FAN_OPEN_CURRENT_THRESHOLD;
extern int DOUBLE_FAN_OPEN_CURRENT_THRESHOLD;

extern int SINGLE_FAN_JAM_CURRENT_THRESHOLD;
extern int DOUBLE_FAN_JAM_CURRENT_THRESHOLD;

extern int SINGLE_FAN_SHORT_CURRENT_THRESHOLD;
extern int DOUBLE_FAN_SHORT_CURRENT_THRESHOLD;





// =============================================
// FanController Class
// =============================================

class FanController {
public:
  FanController();                                            // Constructor
  void begin();                                               // Initialization function
  void controlFan(int fanPort, float desiredSpeed);           // Control fan speed
  void controlFanSimple(int fanPort, float desiredSpeed);     // Control fan speed
  void updateFanConfiguration();
  float readFanCurrent(int shuntPin, int sampleCount = 100);  // Read fan current

  FaultType detectFault(int fanPort, float fanCurrent);  // Fault Detection Methods
  void handleFault(int fanPort, FaultType fault);
  bool isFanFault();

private:
  void setupPins();                                                                                            // Initialize GPIO pins for fans
  void turnOffFan(int fanPort, int pwmPin);                                                                    // Turn off a specific fan
  bool checkSafetyConditions(int fanPort, int pwmPin, float inputVoltage, float fanCurrent, float fanPower);   // Check for safety violations
  int adjustDutyCycle(int fanPort, float desiredSpeed, float fanPower, float inputVoltage, float fanCurrent);  // Adjust PWM duty cycle
  void updateFanStatus(int fanPort, float desiredSpeed, float fanCurrent, float fanPower);                     // Update fan status
  void logFanStatus(int fanPort, float inputVoltage, float fanCurrent, float fanPower, int dutyCycle);         // Log fan status for debugging
  FanConfig getFanConfig(String fanType, String fanProfile);
};

#endif  // FAN_CONTROLLER_H
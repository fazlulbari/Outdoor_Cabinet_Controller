#ifndef MENU_H
#define MENU_H

#include "ODCC_config.h"
#include "Fan_Controller.h"
#include "LCD.h"
#include "Buttons.h"
#include "Storage.h"
// #include "CMBMenu.hpp"
#include "src/CMBMenu.hpp"

#include <STM32FreeRTOS.h>
#include "modbus.h"


#define CORRECT_PIN "0000"
// #define CORRECT_PIN "1001"  // Set your 4-digit password here

extern bool inFactoryResetPrompt;
extern unsigned long factoryResetStartTime;

extern LCD lcd;
extern Buttons buttons;
extern Peripherals peripherals;
extern Storage storage;

extern bool isUserActive;
extern unsigned long lastUserActive;


const char scrollMessage1[] PROGMEM = "SARBS Cabinet Controller";
const char scrollMessage2[] PROGMEM = "SARBS Communications Ltd.";

// Define text to display (stored in PROGMEM)
const char Status[] PROGMEM = { "1. Status" };
const char Temperature[] PROGMEM = { "Temperature" };
const char InputVoltage[] PROGMEM = { "Input Voltage" };

const char Alarms[] PROGMEM = { "2. Alarms" };
const char DoorAlarm[] PROGMEM = { "Door Status" };
const char WaterAlarm[] PROGMEM = { "Water Ingres" };
const char SmokeAlarm[] PROGMEM = { "Smoke Alarm" };
const char TemperatureAlarm[] PROGMEM = { "HI Temp Alarm" };
const char FanFaultAlarm[] PROGMEM = { "Fan Fault" };

const char Settings[] PROGMEM = { "3. Settings" };
const char FanSettings[] PROGMEM = { "Fan Settings  " };
const char TemperatureThresholdL[] PROGMEM = { "Set Lower Temp" };
const char TemperatureThresholdH[] PROGMEM = { "Set Upper Temp" };
const char TempHighThreshold[] PROGMEM = { "Set Higher Temp" };
const char TogglePeriod[] PROGMEM = { "Set Swap Period" };

const char FanModel[] PROGMEM = { "Select Fan Model" };
const char FanProfile[] PROGMEM = { "Set Fan Profile " };
const char FanMode[] PROGMEM = { "Set Fan Mode " };

const char CustomFan[] PROGMEM = { "Custom  Fan " };
const char MaxFanSpeed[] PROGMEM = { "Max Fan Speed " };

const char SingleFanOpenThreshold[] PROGMEM = { "1 Fan Open Curr" };
const char DoubleFanOpenThreshold[] PROGMEM = { "2 Fan Open Curr" };
const char SingleFanJamThreshold[] PROGMEM = { "1 Fan Jam Curr" };
const char DoubleFanJamThreshold[] PROGMEM = { "2 Fan Jam Curr" };
const char SingleFanShortThreshold[] PROGMEM = { "1 Fan Short Curr" };
const char DoubleFanShortThreshold[] PROGMEM = { "2 Fan Short Curr" };

const char FanCurrentUnit[] PROGMEM = { "Fan Current Unit" };

const char SystemSettings[] PROGMEM = { "System Settings" };
const char VoltageThreshold[] PROGMEM = { "Lo Voltage limit" };
const char VoltageHighThreshold[] PROGMEM = { "Hi Voltage limit" };

const char ModbusSettings[] PROGMEM = { "Modbus Settings" };
const char Baudrate[] PROGMEM = { "Baud Rate" };
const char SlaveID[] PROGMEM = { "Slave ID" };

const char FactoryReset[] PROGMEM = { "Factory Reset" };

const char About[] PROGMEM = { "4. About" };
const char SWVersion[] PROGMEM = { "SW Version" };
const char SWDate[] PROGMEM = { "SW Date" };
const char InstalledDate[] PROGMEM = { "Installed Date" };
const char SerialNumber[] PROGMEM = { "Serial Number" };



// Define function IDs
enum MenuFID {
  MenuDummy,
  MenuStatus,
  MenuTemperature,
  MenuInputVoltage,
  MenuAlarms,
  MenuSmokeAlarm,
  MenuTemperatureAlarm,
  MenuFanFaultAlarm,
  MenuWaterAlarm,
  MenuDoorAlarm,
  MenuSettings,
  MenuFanSettings,
  MenuTemperatureThresholdH,
  MenuTemperatureThresholdL,
  MenuTempHighThreshold,
  MenuTogglePeriod,
  MenuFanCurrentUnit,
  MenuFanModel,
  MenuSystemSettings,
  MenuVoltageThreshold,
  MenuVoltageHighThreshold,
  MenuModbusSettings,
  MenuBaudrate,
  MenuSlaveID,
  MenuAbout,
  MenuSWVersion,
  MenuSWDate,
  MenuInstalledDate,
  MenuSerialNumber,
  MenuFanProfile,
  MenuCustomFan,
  MenuMaxFanSpeed,
  MenuSingleFanOpenThreshold,
  MenuDoubleFanOpenThreshold,
  MenuSingleFanJamThreshold,
  MenuDoubleFanJamThreshold,
  MenuSingleFanShortThreshold,
  MenuDoubleFanShortThreshold,


  MenuFactoryReset

};


// Function declarations
void setupMenu();
void showHomeScreen();  // New function for idle home screen
void loopMenu();
void printMenuEntry(const char* f_Info);
void showTemperature();
void showSmokeAlarm();
void showTemperatureAlarm();
void showFanFaultAlarm();
void showWaterAlarm();
void showDoorAlarm();
void showInputVoltage();
void showSettings();
int updateThresholdSetting(int& threshold, const char* settingName, uint8_t eepRomAddress, const char* unit = nullptr, int minLimit = 0, int maxLimit = 255);
int updateThresholdSetting16(int& threshold, const char* settingName, uint8_t eepRomAddress, const char* unit = nullptr, int minLimit = 0, int maxLimit = 512);
void showUpperTemperatureThreshold();
void showLowerTemperatureThreshold();
void showTempHighThreshold();
void showTogglePeriod();
void showFanCurrentUnit();
void showVoltageThreshold();
void showVoltageHighThreshold();
void showBaudrate();
void showFactoryReset();
void showSlaveID();
void showAbout();
void showSWVersion();
void showSWDate();
void showInstalledDate();
void showSerialNumber();

void showMenuFanModel();
void showMenuFanProfile();
void showMaxFanSpeed();

void showSingleFanOpenThreshold();
void showDoubleFanOpenThreshold();
void showSingleFanJamThreshold();
void showDoubleFanJamThreshold();
void showSingleFanShortThreshold();
void showDoubleFanShortThreshold();


bool shouldBlink();


void printRightAlignedInt(LCD& lcd, int value, uint8_t width);

void resetToFactoryDefaults();

bool verifyPassword();



#endif
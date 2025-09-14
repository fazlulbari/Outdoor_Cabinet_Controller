#include "Menu.h"

// Create global CMBMenu instance
CMBMenu<400> Z_Menu;

LCD lcd;
Buttons buttons;

bool isModified;
int lastValue;
bool isUserActive = false;
unsigned long lastUserActive;

// Define About page details
const char SW_VERSION[] PROGMEM = "1.15";
const char SW_DATE[] PROGMEM = "10-05-25";
const char INSTALLED_DATE[] PROGMEM = "10-05-25";
const char SERIAL_NUMBER[] PROGMEM = "SARBS_ODCC_1001";

bool inFactoryResetPrompt = false;
unsigned long factoryResetStartTime = 0;



void showStartupScreen() {
  lcd.clear();
  buttons.tick();  // Call this frequently to detect button presses

  // Check if Up or Down button is pressed
  if (buttons.currentKey == KeyUp || buttons.currentKey == KeyDown) {
    return;
  } else {
    lcd.scrollText(scrollMessage1, 0, 8, 1);  // 1 cycle only
    lcd.setCursor(0, 0);
    lcd.print(" Welcome from ");
    lcd.scrollText(scrollMessage2, 1, 9, 1);  // 1 cycle only

    lcd.clear();

    buttons.tick();  // Call this frequently to detect button presses
  }
}


void setupMenu() {
  lcd.begin();
  buttons.begin();

  DEBUG_PRINTLN("===========================");
  DEBUG_PRINTLN("SARBS Cabinet Controller");
  DEBUG_PRINTLN("===========================");
  DEBUG_PRINTLN("");

  // showStartupScreen();
  lcd.scrollText(scrollMessage1, 0, 8, 1);  // 1 cycle only
  lcd.setCursor(0, 0);
  lcd.print("  Welcome from  ");
  lcd.scrollText(scrollMessage2, 1, 9, 1);  // 1 cycle only

  lcd.clear();



  // Add nodes to menu (layer, string, function ID)
  Z_Menu.addNode(0, Status, MenuStatus);
  Z_Menu.addNode(1, Temperature, MenuTemperature);
  Z_Menu.addNode(1, InputVoltage, MenuInputVoltage);

  Z_Menu.addNode(0, Alarms, MenuAlarms);
  Z_Menu.addNode(1, DoorAlarm, MenuDoorAlarm);
  Z_Menu.addNode(1, WaterAlarm, MenuWaterAlarm);
  Z_Menu.addNode(1, SmokeAlarm, MenuSmokeAlarm);
  Z_Menu.addNode(1, TemperatureAlarm, MenuTemperatureAlarm);
  Z_Menu.addNode(1, FanFaultAlarm, MenuFanFaultAlarm);

  Z_Menu.addNode(0, Settings, MenuSettings);
  Z_Menu.addNode(1, FanSettings, MenuFanSettings);
  Z_Menu.addNode(2, TemperatureThresholdL, MenuTemperatureThresholdL);
  Z_Menu.addNode(2, TemperatureThresholdH, MenuTemperatureThresholdH);
  Z_Menu.addNode(2, TempHighThreshold, MenuTempHighThreshold);
  Z_Menu.addNode(2, TogglePeriod, MenuTogglePeriod);
  Z_Menu.addNode(2, FanProfile, MenuFanProfile);
  Z_Menu.addNode(2, FanModel, MenuFanModel);
  Z_Menu.addNode(2, CustomFan, MenuCustomFan);
  Z_Menu.addNode(3, MaxFanSpeed, MenuMaxFanSpeed);
  Z_Menu.addNode(3, SingleFanOpenThreshold, MenuSingleFanOpenThreshold);
  Z_Menu.addNode(3, DoubleFanOpenThreshold, MenuDoubleFanOpenThreshold);
  Z_Menu.addNode(3, SingleFanJamThreshold, MenuSingleFanJamThreshold);
  Z_Menu.addNode(3, DoubleFanJamThreshold, MenuDoubleFanJamThreshold);
  Z_Menu.addNode(3, SingleFanShortThreshold, MenuSingleFanShortThreshold);
  Z_Menu.addNode(3, DoubleFanShortThreshold, MenuDoubleFanShortThreshold);
  Z_Menu.addNode(2, FanCurrentUnit, MenuFanCurrentUnit);


  Z_Menu.addNode(1, SystemSettings, MenuSystemSettings);
  Z_Menu.addNode(2, VoltageThreshold, MenuVoltageThreshold);
  Z_Menu.addNode(2, VoltageHighThreshold, MenuVoltageHighThreshold);

  Z_Menu.addNode(1, ModbusSettings, MenuModbusSettings);
  Z_Menu.addNode(2, Baudrate, MenuBaudrate);
  Z_Menu.addNode(2, SlaveID, MenuSlaveID);

  Z_Menu.addNode(1, FactoryReset, MenuFactoryReset);

  Z_Menu.addNode(0, About, MenuAbout);
  Z_Menu.addNode(1, SWVersion, MenuSWVersion);
  Z_Menu.addNode(1, SWDate, MenuSWDate);
  Z_Menu.addNode(1, InstalledDate, MenuInstalledDate);
  Z_Menu.addNode(1, SerialNumber, MenuSerialNumber);

  // Build menu and print menu
  const char* info;
  Z_Menu.buildMenu(info);
  // Z_Menu.printMenu();

  // Print current menu entry
  // printMenuEntry(info);
}


const char* getFaultString(FaultType fault) {
  switch (fault) {
    case NO_FAULT: return "No Fault";
    case NO_FAN: return "No Fan  ";
    case FAN_OPEN_1: return "Open One";
    case FAN_OPEN_2: return "Open Two";

    case FAN_JAM: return "Jammed  ";
    case FAN_JAM_1: return "Jam One ";
    case FAN_JAM_2: return "Jam Two";
    case FAN_SHORT: return "Shorted ";
    case FAN_SHORT_1: return "Short 1 ";
    case FAN_SHORT_2: return "Short 2 ";
    default: return "Unknown ";
  }
}


void showHomeScreen() {
  static unsigned long lastUpdate = 0;
  static unsigned long lastButtonPress = 0;  // Track last button press time
  static int page = 0;
  static bool autoRotationEnabled = true;  // Auto rotation enabled by default

  // Static variables to store last displayed values
  static float lastInputVoltage = -1;
  static float lastTemperature = -1;
  static bool lastFanPort1Status = false;
  static bool lastFanPort2Status = false;
  static bool lastFanPort1Fault = false;
  static bool lastFanPort2Fault = false;
  static float lastFanPort1Current = -1;
  static float lastFanPort2Current = -1;
  static float lastFanPort1Power = -1;
  static float lastFanPort2Power = -1;

  buttons.tick();  // Call this frequently to detect button presses

  // Check if Up or Down button is pressed
  if (buttons.currentKey == KeyUp || buttons.currentKey == KeyDown) {
    autoRotationEnabled = false;  // Disable auto rotation
    lastButtonPress = millis();   // Update last button press time

    switch (buttons.currentKey) {
      case KeyUp:
        page = (page - 1 + 4) % 4;  // Cycle to previous page
        break;
      case KeyDown:
        page = (page + 1) % 4;  // Cycle to next page
        break;
      default:
        break;
    }

    buttons.currentKey = KeyNone;  // Reset button state
  } else if (buttons.currentKey == KeyEnter || buttons.currentKey == KeyEsc) {
    autoRotationEnabled = true;    // Re-enable auto rotation
    lastButtonPress = millis();    // Update last button press time
    buttons.currentKey = KeyNone;  // Reset button state
  }

  // Re-enable auto rotation if no button is pressed for 60 seconds
  if (!autoRotationEnabled && millis() - lastButtonPress >= 60 * 1000) {
    autoRotationEnabled = true;  // Re-enable auto rotation
  }
  // Turn Off Backlight if User inactive for 30 seconds
  if (isUserActive && millis() - lastUserActive >= 60 * 1000) {
    isUserActive = false;  // User not active
  }

  // Auto rotate pages every 5 seconds if auto rotation is enabled
  if (autoRotationEnabled && millis() - lastUpdate >= 5 * 1000) {
    lastUpdate = millis();
    page = (page + 1) % 4;  // Cycle to next page
  }

  // Display the current page
  static int lastPage = -1;  // Store the last displayed page to prevent unnecessary updates

  if (page != lastPage) {
    // lcd.clear();
    lastPage = page;
    // Reset last displayed values when page changes
    lastInputVoltage = -1;
    lastTemperature = -1;
    lastFanPort1Status = false;
    lastFanPort2Status = false;
    lastFanPort1Fault = FanPort1_fault;
    lastFanPort2Fault = FanPort2_fault;
    lastFanPort1Current = -1;
    lastFanPort2Current = -1;
    lastFanPort1Power = -1;
    lastFanPort2Power = -1;
  }

  switch (page) {
    case 0:
      // Page 1: System voltage and temperature
      if (inputVoltage != lastInputVoltage || temperature != lastTemperature) {
        lcd.setCursor(0, 0);
        lcd.print("Input: ");

        if (inputVoltage < 10) {
          lcd.print("LOW      ");
        } else {
          lcd.print(inputVoltage, 2);
          lcd.print(" V  ");
        }
        lcd.setCursor(0, 1);
        if (temperature >= TEMP_HIGH_THRESHOLD + 11) {
          lcd.print("No temp Sensor  ");
        } else {
          lcd.print("Temp : ");
          lcd.print(temperature, 2);
          lcd.print(" ");
          lcd.print((char)223);  // 223 is the ASCII code for the degree symbol on many LCDs
          lcd.print("C ");
        }
        lastInputVoltage = inputVoltage;
        lastTemperature = temperature;
      }
      break;

    case 1:
      // Page 2: Fan status and fault
      // if (FanPort1_status != lastFanPort1Status || FanPort2_status != lastFanPort2Status || FanPort1_fault != lastFanPort1Fault || FanPort2_fault != lastFanPort2Fault) {
      if (1) {
        lcd.setCursor(0, 0);
        lcd.print("Fan 1 : ");
        if (FanPort1_fault) {
          lcd.print(getFaultString(FanPort1_fault_Status));
        } else {
          lcd.print(FanPort1_status ? "Running  " : "Idle    ");
        }

        lcd.setCursor(0, 1);
        lcd.print("Fan 2 : ");
        if (FanPort2_fault) {
          lcd.print(getFaultString(FanPort2_fault_Status));
        } else {
          lcd.print(FanPort2_status ? "Running  " : "Idle    ");
        }

        lastFanPort1Status = FanPort1_status;
        lastFanPort2Status = FanPort2_status;
        lastFanPort1Fault = FanPort1_fault;
        lastFanPort2Fault = FanPort2_fault;
      }
      break;
    case 2:
      // Page 3: Fan currents
      if (FanPort1_current != lastFanPort1Current || FanPort2_current != lastFanPort2Current) {
        lcd.setCursor(0, 0);
        lcd.print("Fan 1 C: ");
        if (Fan_Current_Unit) {
          lcd.print(FanPort1_current / 1000, 2);
          lcd.print(" A ");
        } else {
          int FanPort1_mA = FanPort1_current;
          printRightAlignedInt(lcd, FanPort1_mA, 3);  // 3-character field
          lcd.setCursor(12, 0);
          lcd.print(" mA ");
        }

        lcd.setCursor(0, 1);
        lcd.print("Fan 2 C: ");
        if (Fan_Current_Unit) {
          lcd.print(FanPort2_current / 1000, 2);
          lcd.print(" A ");
        } else {
          int FanPort2_mA = FanPort2_current;
          printRightAlignedInt(lcd, FanPort2_mA, 3);
          lcd.setCursor(12, 1);
          lcd.print(" mA ");
        }

        lastFanPort1Current = FanPort1_current;
        lastFanPort2Current = FanPort2_current;
      }
      break;


    case 3:
      // Page 4: Fan power
      if (FanPort1_power != lastFanPort1Power || FanPort2_power != lastFanPort2Power) {
        lcd.setCursor(0, 0);
        lcd.print("Fan 1 P: ");
        lcd.print(FanPort1_power, 2);
        lcd.print(" W ");

        lcd.setCursor(0, 1);
        lcd.print("Fan 2 P: ");
        lcd.print(FanPort2_power, 2);
        lcd.print(" W ");

        lastFanPort1Power = FanPort1_power;
        lastFanPort2Power = FanPort2_power;
      }
      break;

    default:
      break;
  }
}

void loopMenu() {
  static bool inMenuMode = false;
  isModified = false;
  int fid = 0;
  lastValue = -1;  // Store the last displayed value
  buttons.begin();
  buttons.tick();

  // if (isUserActive) {
  //   lcd.backlight(255);
  // } else {
  //   lcd.backlight(0);
  // }

  lcd.controlBacklight(isUserActive);

  if (!inMenuMode && KeyMenuEnter == buttons.currentKey) {
    inMenuMode = true;  // Enter menu mode

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" Entering Menu ");
    // lcd.print("    Entering    ");
    // lcd.setCursor(0, 1);
    // lcd.print("      Menu      ");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    buttons.currentKey = KeyNone;  // Reset key

    Z_Menu.resetMenu();  // 🛠️ Resets menu to top

    // Set the menu to the first item and display it
    Z_Menu.setMenuNode(0);  // Set to the first menu item
    const char* info;
    Z_Menu.getInfo(info);  // Get the info for the first menu item
    printMenuEntry(info);  // Display the first menu item
  }

  if (inMenuMode) {
    // Handle menu navigation
    fid = 0;
    const char* info;
    bool layerChanged = false;

    buttons.tick();

    switch (buttons.currentKey) {
      case KeyExit:
        // Exit menu mode only if at the top level
        if (Z_Menu.getCurrentLayer() > 0) {
          DEBUG_PRINTLN("Exiting submenu");
          Z_Menu.exit();  // Go back to the previous menu layer
          lcd.setCursor(0, 1);
          lcd.print("                ");
          vTaskDelay(100 / portTICK_PERIOD_MS);  // optional
        } else {
          inMenuMode = false;  // Exit menu mode
          DEBUG_PRINTLN("Exiting menu");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("  Exiting Menu ");
          // lcd.print("    Exiting    ");
          // lcd.setCursor(0, 1);
          // lcd.print("      Menu      ");
          vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        break;
      case KeyEnter:
        {
          const char* info;
          int currentFID = Z_Menu.getInfo(info);

          if (currentFID == MenuSettings) {
            // if (1) {
            if (verifyPassword()) {
              Z_Menu.enter(layerChanged);  // Only enter Settings if password correct
              DEBUG_PRINTLN("Access granted to Settings menu");

              Z_Menu.getInfo(info);
              printMenuEntry(info);
            } else {
              DEBUG_PRINTLN("Access denied to Settings menu or cancelled");

              // 🛠️ Important: reprint the menu item you were on
              Z_Menu.getInfo(info);
              printMenuEntry(info);

              buttons.currentKey = KeyNone;  // Clear key
            }
          } else {
            Z_Menu.enter(layerChanged);  // Normal menu entry for other items
            DEBUG_PRINTLN("Entering submenu");

            Z_Menu.getInfo(info);
            printMenuEntry(info);
          }
          break;
        }
      case KeyDown:
        Z_Menu.right();
        DEBUG_PRINTLN("Navigating Down");
        break;
      case KeyUp:
        Z_Menu.left();
        DEBUG_PRINTLN("Navigating Up");
        break;
      default:
        break;
    }

    if (KeyNone != buttons.currentKey) {
      fid = Z_Menu.getInfo(info);
      DEBUG_PRINT("Current Function ID: ");
      DEBUG_PRINTLN(fid);
      printMenuEntry(info);

      if ((0 != fid) && (KeyEnter == buttons.currentKey) && (!layerChanged)) {
        DEBUG_PRINT("Executing function for ID: ");
        DEBUG_PRINTLN(fid);

        while (KeyExit != buttons.currentKey) {
          buttons.tick();


          if (buttons.currentKey == KeyExit) {
            lcd.setCursor(0, 1);
            lcd.print("                ");  // Clear second line when exiting

            DEBUG_PRINTLN("Exiting submenu");
            break;
          }

          switch (fid) {
            case MenuTemperature:
              showTemperature();
              break;
            case MenuSmokeAlarm:
              showSmokeAlarm();
              break;
            case MenuTemperatureAlarm:
              showTemperatureAlarm();
              break;
            case MenuFanFaultAlarm:
              showFanFaultAlarm();
              break;
            case MenuWaterAlarm:
              showWaterAlarm();
              break;
            case MenuDoorAlarm:
              showDoorAlarm();
              break;
            case MenuSettings:
              showSettings();
              break;
            case MenuTemperatureThresholdH:
              showUpperTemperatureThreshold();
              break;
            case MenuTemperatureThresholdL:
              showLowerTemperatureThreshold();
              break;
            case MenuTempHighThreshold:
              showTempHighThreshold();
              break;
            case MenuTogglePeriod:
              showTogglePeriod();
              break;
            case MenuFanCurrentUnit:
              showFanCurrentUnit();
              break;
            case MenuFanModel:
              showMenuFanModel();
              break;
            case MenuFanProfile:
              showMenuFanProfile();
              break;
            case MenuMaxFanSpeed:
              showMaxFanSpeed();
              break;
            case MenuSingleFanOpenThreshold:
              showSingleFanOpenThreshold();
              break;
            case MenuDoubleFanOpenThreshold:
              showDoubleFanOpenThreshold();
              break;
            case MenuSingleFanJamThreshold:
              showSingleFanJamThreshold();
              break;
            case MenuDoubleFanJamThreshold:
              showDoubleFanJamThreshold();
              break;
            case MenuSingleFanShortThreshold:
              showSingleFanShortThreshold();
              break;
            case MenuDoubleFanShortThreshold:
              showDoubleFanShortThreshold();
              break;
            case MenuVoltageThreshold:
              showVoltageThreshold();
              break;
            case MenuVoltageHighThreshold:
              showVoltageHighThreshold();
              break;
            case MenuBaudrate:
              showBaudrate();
              break;
            case MenuSlaveID:
              showSlaveID();
              break;
            case MenuAbout:
              showAbout();
              break;
            case MenuSWVersion:
              showSWVersion();
              break;
            case MenuSWDate:
              showSWDate();
              break;
            case MenuInstalledDate:
              showInstalledDate();
              break;
            case MenuSerialNumber:
              showSerialNumber();
              break;
            case MenuInputVoltage:
              showInputVoltage();
              break;
            case MenuFactoryReset:
              if (!inFactoryResetPrompt) {
                lcd.setCursor(0, 1);
                lcd.print("DoubleClick=Yes ");
                factoryResetStartTime = millis();
                inFactoryResetPrompt = true;
                buttons.currentKey = KeyNone;
              } else {
                showFactoryReset();  // Non-blocking update
              }
              break;


            default:
              break;
          }
          vTaskDelay(5 / portTICK_PERIOD_MS);  // Small delay
        }
      } else {
        //
      }
      buttons.currentKey = KeyNone;
    }
  } else {
    // // If idle for over 60 seconds, show startup screen again
    // if (!isUserActive && millis() - lastUserActive >= 6 * 1000) {
    //   showStartupScreen();
    //   lastUserActive = millis();  // reset idle timer after showing animation
    // } else {
    showHomeScreen();
    // }

    vTaskDelay(50 / portTICK_PERIOD_MS);  // Small delay to reduce flickering
  }
}

void printMenuEntry(const char* f_Info) {
  String info_s;
  MBHelper::stringFromPgm(f_Info, info_s);

  // Print on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(info_s);
}


void showInputVoltage() {
  lcd.setCursor(0, 1);
  if (inputVoltage < 10) {
    lcd.print("LOW VOLTAGE     ");
  } else {
    lcd.print(inputVoltage);
    lcd.print(" V");
  }
}

void showTemperature() {
  lcd.setCursor(0, 1);
  if (temperature < 0) {
    lcd.print("No Sensor");
  } else {
    lcd.print(temperature);
    lcd.print((char)223);  // 223 is the ASCII code for the degree symbol on many LCDs
    lcd.print("C ");
  }
}


void showSettings() {
  lcd.setCursor(0, 1);
  lcd.print("Settings Menu");
}



void showDoorAlarm() {
  lcd.setCursor(0, 1);
  lcd.print(Door_open_alarm);
}

void showWaterAlarm() {
  lcd.setCursor(0, 1);
  lcd.print(Water_ingres_alarm);
}

void showSmokeAlarm() {
  lcd.setCursor(0, 1);
  lcd.print(String(Smoke_detected_alarm));
}

void showTemperatureAlarm() {
  lcd.setCursor(0, 1);
  lcd.print(String(High_temperature_alarm));
}

void showFanFaultAlarm() {
  lcd.setCursor(0, 1);
  lcd.print(Fan_fault_alarm);
}

void showLowerTemperatureThreshold() {
  // Ensure the lower threshold is always less than the upper threshold by at least 1
  LOWER_TEMP_THRESHOLD = updateThresholdSetting(LOWER_TEMP_THRESHOLD, "Temp Threshold L", LOWER_TEMP_THRESHOLD_ADDR, "\xDF"
                                                                                                                     "C",
                                                20, 30);
                                                // 20, UPPER_TEMP_THRESHOLD - 1);
  holdingRegisters[REG_LOWER_THRESHOLD] = LOWER_TEMP_THRESHOLD;
}


void showUpperTemperatureThreshold() {
  // Ensure the upper threshold is always greater than the lower threshold by at least 1
  UPPER_TEMP_THRESHOLD = updateThresholdSetting(UPPER_TEMP_THRESHOLD, "Temp Threshold H", UPPER_TEMP_THRESHOLD_ADDR, "\xDF"
                                                                                                                     "C",
                                                20, 40);
                                                // LOWER_TEMP_THRESHOLD + 1, TEMP_HIGH_THRESHOLD - 1);
  holdingRegisters[REG_UPPER_THRESHOLD] = UPPER_TEMP_THRESHOLD;
}


void showTempHighThreshold() {
  // Ensure the high threshold is always greater than the upper threshold by at least 1
  TEMP_HIGH_THRESHOLD = updateThresholdSetting(TEMP_HIGH_THRESHOLD, "Temp High Threshold", TEMP_HIGH_THRESHOLD_ADDR, "\xDF"
                                                                                                                     "C",
                                               20, 50);
                                              //  UPPER_TEMP_THRESHOLD + 1, 60);
  holdingRegisters[REG_HIGH_THRESHOLD] = TEMP_HIGH_THRESHOLD;
}

void showMaxFanSpeed() {
  CUSTOM_MAX_FAN_SPEED = updateThresholdSetting(CUSTOM_MAX_FAN_SPEED, "Max Fan Speed", CUSTOM_MAX_FAN_SPEED_ADDR);
  holdingRegisters[REG_CUSTOM_MAX_FAN_SPEED] = CUSTOM_MAX_FAN_SPEED;
}

void showSingleFanOpenThreshold() {
  SINGLE_FAN_OPEN_CURRENT_THRESHOLD = updateThresholdSetting16(SINGLE_FAN_OPEN_CURRENT_THRESHOLD, "SF Open", SINGLE_FAN_OPEN_THRESHOLD_ADDR, "mA");
  holdingRegisters[REG_SF_OPEN_THRESHOLD] = SINGLE_FAN_OPEN_CURRENT_THRESHOLD;
}

void showDoubleFanOpenThreshold() {
  DOUBLE_FAN_OPEN_CURRENT_THRESHOLD = updateThresholdSetting16(DOUBLE_FAN_OPEN_CURRENT_THRESHOLD, "DF Open", DOUBLE_FAN_OPEN_THRESHOLD_ADDR, "mA");
  holdingRegisters[REG_DF_OPEN_THRESHOLD] = DOUBLE_FAN_OPEN_CURRENT_THRESHOLD;
}

void showSingleFanJamThreshold() {
  SINGLE_FAN_JAM_CURRENT_THRESHOLD = updateThresholdSetting16(SINGLE_FAN_JAM_CURRENT_THRESHOLD, "SF Jam", SINGLE_FAN_JAM_THRESHOLD_ADDR, "mA");
  holdingRegisters[REG_SF_JAM_THRESHOLD] = SINGLE_FAN_JAM_CURRENT_THRESHOLD;
}

void showDoubleFanJamThreshold() {
  DOUBLE_FAN_JAM_CURRENT_THRESHOLD = updateThresholdSetting16(DOUBLE_FAN_JAM_CURRENT_THRESHOLD, "DF Jam", DOUBLE_FAN_JAM_THRESHOLD_ADDR, "mA");
  holdingRegisters[REG_DF_JAM_THRESHOLD] = DOUBLE_FAN_JAM_CURRENT_THRESHOLD;
}

void showSingleFanShortThreshold() {
  SINGLE_FAN_SHORT_CURRENT_THRESHOLD = updateThresholdSetting16(SINGLE_FAN_SHORT_CURRENT_THRESHOLD, "SF Short", SINGLE_FAN_SHORT_THRESHOLD_ADDR, "mA");
  holdingRegisters[REG_SF_SHORT_THRESHOLD] = SINGLE_FAN_SHORT_CURRENT_THRESHOLD;
}

void showDoubleFanShortThreshold() {
  DOUBLE_FAN_SHORT_CURRENT_THRESHOLD = updateThresholdSetting16(DOUBLE_FAN_SHORT_CURRENT_THRESHOLD, "DF Short", DOUBLE_FAN_SHORT_THRESHOLD_ADDR, "mA");
  holdingRegisters[REG_DF_SHORT_THRESHOLD] = DOUBLE_FAN_SHORT_CURRENT_THRESHOLD;
}

void showTogglePeriod() {
  TOGGLE_PERIOD = updateThresholdSetting(TOGGLE_PERIOD, "Toggle Period", TOGGLE_PERIOD_ADDR, "min");
  holdingRegisters[REG_TOGGLE_PERIOD] = TOGGLE_PERIOD;
}

void showFanCurrentUnit() {
  lcd.setCursor(0, 0);
  lcd.print("Fan Current Unit");

  // Toggle unit on button press
  if (KeyEnter == buttons.currentKey) {
    Fan_Current_Unit = !Fan_Current_Unit;  // Toggle unit
    buttons.currentKey = KeyNone;          // Reset button state
  }

  // Display the current unit
  lcd.setCursor(0, 1);
  lcd.print(Fan_Current_Unit ? " Ampere     " : " milliAmpere");
}



void showMenuFanProfile() {
  static int fanIndex = 0;
  static bool lastBlinkState = false;

  if (!isModified) {
    for (int i = 0; i < NUM_FAN_MODE; i++) {
      if (FAN_PROFILE == FAN_MODE[i]) {
        fanIndex = i;
        break;
      }
    }
    isModified = true;
  }

  buttons.begin();
  buttons.beginDoubleClick();

  if (KeyDown == buttons.currentKey) {
    buttons.currentKey = KeyNone;
    fanIndex = (fanIndex + 1) % NUM_FAN_MODE;
  } else if (KeyUp == buttons.currentKey) {
    buttons.currentKey = KeyNone;
    fanIndex = (fanIndex - 1 + NUM_FAN_MODE) % NUM_FAN_MODE;
  } else if (DoubleClick == buttons.currentKey) {
    FAN_PROFILE = FAN_MODE[fanIndex];
    lcd.setCursor(0, 1);
    lcd.print("Saved!  ");
    holdingRegisters[REG_FAN_PROFILE] = fanIndex;
    peripherals.confirmBeep();  // Confirmation sound
    vTaskDelay(100 / portTICK_PERIOD_MS);
    peripherals.confirmBeep();  // Confirmation sound
    vTaskDelay(700 / portTICK_PERIOD_MS);
    lcd.setCursor(0, 1);
    lcd.print(FAN_MODE[fanIndex]);
    buttons.currentKey = KeyNone;
  }

  bool blink = shouldBlink();
  if (fanIndex != lastValue || blink != lastBlinkState) {
    lcd.setCursor(0, 1);
    lcd.print("         ");
    lcd.setCursor(0, 1);
    if (blink) {
      lcd.print("   ");  // blink effect
    } else {
      lcd.print(FAN_MODE[fanIndex]);
    }
    lastValue = fanIndex;
    lastBlinkState = blink;
  }
}

void showMenuFanModel() {
  static int fanIndex = 0;
  static bool lastBlinkState = false;

  if (!isModified) {
    for (int i = 0; i < NUM_FAN_MODEL; i++) {
      if (FAN_TYPE == FAN_MODEL[i]) {
        fanIndex = i;
        break;
      }
    }
    isModified = true;
  }

  buttons.begin();
  buttons.beginDoubleClick();

  if (KeyDown == buttons.currentKey) {
    buttons.currentKey = KeyNone;
    fanIndex = (fanIndex + 1) % NUM_FAN_MODEL;
  } else if (KeyUp == buttons.currentKey) {
    buttons.currentKey = KeyNone;
    fanIndex = (fanIndex - 1 + NUM_FAN_MODEL) % NUM_FAN_MODEL;
  } else if (DoubleClick == buttons.currentKey) {
    FAN_TYPE = FAN_MODEL[fanIndex];
    lcd.setCursor(0, 1);
    lcd.print("Saved!  ");
    holdingRegisters[REG_FAN_TYPE] = fanIndex;
    peripherals.confirmBeep();  // Confirmation sound
    vTaskDelay(100 / portTICK_PERIOD_MS);
    peripherals.confirmBeep();  // Confirmation sound
    vTaskDelay(700 / portTICK_PERIOD_MS);
    lcd.setCursor(0, 1);
    lcd.print(FAN_MODEL[fanIndex]);
    buttons.currentKey = KeyNone;
  }

  bool blink = shouldBlink();
  if (fanIndex != lastValue || blink != lastBlinkState) {
    lcd.setCursor(0, 1);
    lcd.print("         ");
    lcd.setCursor(0, 1);
    if (blink) {
      lcd.print("   ");
    } else {
      lcd.print(FAN_MODEL[fanIndex]);
    }
    lastValue = fanIndex;
    lastBlinkState = blink;
  }
}


void showVoltageThreshold() {
  VOLTAGE_LOW_THRESHOLD = updateThresholdSetting(VOLTAGE_LOW_THRESHOLD, "Voltage Threshold", VOLTAGE_LOW_THRESHOLD_ADDR, " V", 40, 48);
  holdingRegisters[REG_VOLTAGE_LOW] = VOLTAGE_LOW_THRESHOLD;
}

void showVoltageHighThreshold() {
  VOLTAGE_HIGH_THRESHOLD = updateThresholdSetting(VOLTAGE_HIGH_THRESHOLD, "Voltage High Threshold", VOLTAGE_HIGH_THRESHOLD_ADDR, " V", 50, 58);
  holdingRegisters[REG_VOLTAGE_HIGH] = VOLTAGE_HIGH_THRESHOLD;
}

void showBaudrate() {
  static int baudRateIndex = 0;  // Index of the current baud rate in the list

  if (!isModified) {
    // Find the current baud rate in the list
    for (int i = 0; i < NUM_BAUD_RATES; i++) {
      if (Baud_Rate == BAUD_RATES[i]) {
        baudRateIndex = i;
        break;
      }
    }
    isModified = true;
  }

  // Handle button presses
  buttons.begin();             // Enable long press functionality
  buttons.beginDoubleClick();  // Enable double-click functionality

  if (KeyDown == buttons.currentKey) {
    buttons.currentKey = KeyNone;
    // Move to the next baud rate
    baudRateIndex = (baudRateIndex + 1) % NUM_BAUD_RATES;
    DEBUG_PRINT("Selected Baud Rate: ");
    DEBUG_PRINTLN(Baud_Rate);
  } else if (KeyUp == buttons.currentKey) {
    buttons.currentKey = KeyNone;
    // Move to the previous baud rate
    baudRateIndex = (baudRateIndex - 1 + NUM_BAUD_RATES) % NUM_BAUD_RATES;
    DEBUG_PRINT("Selected Baud Rate: ");
    DEBUG_PRINTLN(Baud_Rate);
  } else if (DoubleClick == buttons.currentKey) {
    // Save the selected baud rate
    Baud_Rate = BAUD_RATES[baudRateIndex];
    lcd.setCursor(0, 1);
    lcd.print("Saved! ");
    holdingRegisters[REG_Baud_Rate] = baudRateIndex;
    peripherals.confirmBeep();  // Confirmation sound
    vTaskDelay(100 / portTICK_PERIOD_MS);
    peripherals.confirmBeep();  // Confirmation sound
    vTaskDelay(700 / portTICK_PERIOD_MS);

    lcd.setCursor(0, 1);
    lcd.print("         ");  // Clear the row
    buttons.currentKey = KeyNone;
  }

  // Update the display only if the baud rate has changed
  if (baudRateIndex != lastValue) {
    lcd.setCursor(0, 1);
    lcd.print("         ");  // Clear the row before printing
    lcd.setCursor(0, 1);
    lcd.print(BAUD_RATES[baudRateIndex]);
    DEBUG_PRINT("Selected Baud Rate: ");
    DEBUG_PRINTLN(BAUD_RATES[baudRateIndex]);
    lastValue = baudRateIndex;
  }
}

void showSlaveID() {
  Slave_id = updateThresholdSetting(Slave_id, "Slave ID", SLAVE_ID_ADDR);
}

void showAbout() {
  lcd.setCursor(0, 1);
  lcd.print("About System");
}

void showSWVersion() {
  lcd.setCursor(0, 1);
  lcd.print("v. ");
  lcd.print(SW_VERSION);
}

void showSWDate() {
  lcd.setCursor(0, 1);
  lcd.print(SW_DATE);
  lcd.print("    ");
}

void showInstalledDate() {
  lcd.setCursor(0, 1);
  lcd.print(INSTALLED_DATE);
  lcd.print("    ");
}

void showSerialNumber() {
  lcd.setCursor(0, 1);
  lcd.print(SERIAL_NUMBER);
  lcd.print("    ");
}


void showFactoryReset() {
  static bool shown = false;
  static bool resetDone = false;
  static unsigned long resetTime = 0;

  buttons.begin();
  buttons.beginDoubleClick();

  // Show initial message once
  if (!shown) {
    lcd.setCursor(0, 1);
    lcd.print("Reset Settings ?");
    shown = true;
    resetDone = false;
  }

  // If reset confirmed
  if (!resetDone && DoubleClick == buttons.currentKey) {
    resetToFactoryDefaults();
    peripherals.confirmBeep();  // Confirmation sound
    vTaskDelay(100 / portTICK_PERIOD_MS);
    peripherals.confirmBeep();  // Confirmation sound
    lcd.setCursor(0, 1);
    lcd.print("Reset Complete! ");
    resetTime = millis();
    resetDone = true;
    buttons.currentKey = KeyNone;
  }

  // After a short delay, restore the original prompt
  if (resetDone && millis() - resetTime >= 3000) {
    lcd.setCursor(0, 1);
    lcd.print("Reset Settings ?");
    resetDone = false;
  }
}


void printRightAlignedInt(LCD& lcd, int value, uint8_t width) {
  int temp = value;
  uint8_t digits = 1;
  while (temp >= 10) {
    temp /= 10;
    digits++;
  }
  for (uint8_t i = 0; i < (width - digits); i++) {
    lcd.print(' ');
  }
  lcd.print(value);
}



bool shouldBlink() {
  static unsigned long lastBlink = 0;
  static bool blinkState = false;

  unsigned long now = millis();
  unsigned long interval = blinkState ? 200 : 400;  // OFF for 200ms, ON for 400ms

  if (now - lastBlink >= interval) {
    blinkState = !blinkState;
    lastBlink = now;
  }
  return blinkState;
}



int updateThresholdSetting(int& threshold, const char* settingName, uint8_t eepRomAddress, const char* unit, int minLimit, int maxLimit) {
  static bool lastBlinkState = false;

  if (!isModified) {
    threshold = constrain(threshold, minLimit, maxLimit);
    buttons.value = threshold;
    isModified = true;
  }

  buttons.beginLongPress();
  buttons.beginDoubleClick();
  buttons.value = constrain(buttons.value, minLimit, maxLimit);

  if (DoubleClick == buttons.currentKey) {
    threshold = buttons.value;
    lcd.setCursor(0, 1);
    lcd.print("Saved! ");
    peripherals.confirmBeep();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    peripherals.confirmBeep();
    vTaskDelay(700 / portTICK_PERIOD_MS);
    buttons.currentKey = KeyNone;
    lastValue = -1;
  }

  bool blink = shouldBlink();
  if (buttons.value != lastValue || blink != lastBlinkState) {
    lcd.setCursor(0, 1);
    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%3d", buttons.value);

    if (blink) {
      lcd.print("   ");  // Blink spaces
    } else {
      lcd.print(buffer);
    }

    if (unit) {
      lcd.print(" ");
      lcd.print(unit);  // Keep unit constant
    }

    lastValue = buttons.value;
    lastBlinkState = blink;
  }

  return threshold;
}




int updateThresholdSetting16(int& threshold, const char* settingName, uint8_t eepRomAddress, const char* unit, int minLimit, int maxLimit) {
  static bool lastBlinkState = false;

  if (!isModified) {
    threshold = constrain(threshold, minLimit, maxLimit);
    buttons.value = threshold;
    isModified = true;
  }

  buttons.beginLongPress();
  buttons.beginDoubleClick();
  buttons.value = constrain(buttons.value, minLimit, maxLimit);

  if (DoubleClick == buttons.currentKey) {
    threshold = buttons.value;
    lcd.setCursor(0, 1);
    lcd.print("Saved! ");
    peripherals.confirmBeep();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    peripherals.confirmBeep();
    vTaskDelay(700 / portTICK_PERIOD_MS);
    buttons.currentKey = KeyNone;
    lastValue = -1;
  }

  bool blink = shouldBlink();
  if (buttons.value != lastValue || blink != lastBlinkState) {
    lcd.setCursor(0, 1);
    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%3d", buttons.value);

    if (blink) {
      lcd.print("   ");
    } else {
      lcd.print(buffer);
    }

    if (unit) {
      lcd.print(" ");
      lcd.print(unit);  // Do not blink the unit
    }

    lastValue = buttons.value;
    lastBlinkState = blink;
  }

  return threshold;
}




bool verifyPassword() {
  char enteredPin[5] = "0000";  // 4 digits + null terminator
  int currentDigit = 0;
  bool editing = true;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter PIN:");

  buttons.currentKey = KeyNone;  // Clear old button presses

  while (editing) {
    buttons.tick();

    // Display current PIN with blinking digit
    lcd.setCursor(0, 1);
    for (int i = 0; i < 4; i++) {
      if (i == currentDigit && shouldBlink()) {
        lcd.print(' ');  // Blink the current digit
      } else {
        lcd.print(enteredPin[i]);
      }
    }

    // Handle button inputs
    if (buttons.currentKey == KeyUp) {
      enteredPin[currentDigit]++;
      if (enteredPin[currentDigit] > '9') enteredPin[currentDigit] = '0';
      buttons.currentKey = KeyNone;
    } else if (buttons.currentKey == KeyDown) {
      enteredPin[currentDigit]--;
      if (enteredPin[currentDigit] < '0') enteredPin[currentDigit] = '9';
      buttons.currentKey = KeyNone;
    } else if (buttons.currentKey == KeyEnter) {
      currentDigit = (currentDigit + 1) % 4;
      buttons.currentKey = KeyNone;
    } else if (buttons.currentKey == KeyMenuEnter) {
      enteredPin[4] = '\0';  // Null terminate
      editing = false;
      buttons.currentKey = KeyNone;
    } else if (buttons.currentKey == KeyExit) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("   Cancelled");
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      buttons.currentKey = KeyNone;
      return false;
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }


  // Password check
  if (strcmp(enteredPin, CORRECT_PIN) == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" Access Granted");
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Short wait
    return true;
  } else {
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print(" Access Denied");

    // Flash backlight for wrong password
    lcd.backlightFlash();
    vTaskDelay(700 / portTICK_PERIOD_MS);  // Short wait

    buttons.currentKey = KeyNone;  // Clear key
    return false;
  }
}



void resetToFactoryDefaults() {
  // ───── Temperature Settings ─────

  // Lab Testing Values
  // LOWER_TEMP_THRESHOLD = 24;
  // UPPER_TEMP_THRESHOLD = 25;
  // TEMP_HIGH_THRESHOLD = 26;

   // Site Install Values
  LOWER_TEMP_THRESHOLD = 27;
  UPPER_TEMP_THRESHOLD = 32;
  TEMP_HIGH_THRESHOLD  = 45;

  // ───── Fan Swap Time (Toggle Period) ─────
  TOGGLE_PERIOD = 30;  // e.g., 30 Minutes — set your default here

  // ───── Voltage Settings ─────
  VOLTAGE_LOW_THRESHOLD = 45;
  VOLTAGE_HIGH_THRESHOLD = 56;

  // ───── Fan Settings ─────
  CUSTOM_MAX_FAN_SPEED = 200;

  DOUBLE_FAN_OPEN_CURRENT_THRESHOLD = 100;
  SINGLE_FAN_OPEN_CURRENT_THRESHOLD = 200;
  SINGLE_FAN_JAM_CURRENT_THRESHOLD = 320;
  DOUBLE_FAN_JAM_CURRENT_THRESHOLD = 370;
  SINGLE_FAN_SHORT_CURRENT_THRESHOLD = 420;
  DOUBLE_FAN_SHORT_CURRENT_THRESHOLD = 470;

  // ───── Communication Settings ─────
  uint8_t fanModelIndex = 0;
  uint8_t fanModeIndex = 0;
  uint8_t baudRateIndex = 0;
  Slave_id = 11;

  FAN_TYPE = FAN_MODEL[fanModelIndex];
  FAN_PROFILE = FAN_MODE[fanModeIndex];
  Baud_Rate = BAUD_RATES[baudRateIndex];

  // ───── Modbus Register Sync ─────
  holdingRegisters[REG_LOWER_THRESHOLD] = LOWER_TEMP_THRESHOLD;
  holdingRegisters[REG_UPPER_THRESHOLD] = UPPER_TEMP_THRESHOLD;
  holdingRegisters[REG_HIGH_THRESHOLD] = TEMP_HIGH_THRESHOLD;
  holdingRegisters[REG_TOGGLE_PERIOD] = TOGGLE_PERIOD;

  holdingRegisters[REG_CUSTOM_MAX_FAN_SPEED] = CUSTOM_MAX_FAN_SPEED;

  holdingRegisters[REG_DF_OPEN_THRESHOLD] = DOUBLE_FAN_OPEN_CURRENT_THRESHOLD;
  holdingRegisters[REG_SF_OPEN_THRESHOLD] = SINGLE_FAN_OPEN_CURRENT_THRESHOLD;
  holdingRegisters[REG_SF_JAM_THRESHOLD] = SINGLE_FAN_JAM_CURRENT_THRESHOLD;
  holdingRegisters[REG_DF_JAM_THRESHOLD] = DOUBLE_FAN_JAM_CURRENT_THRESHOLD;
  holdingRegisters[REG_SF_SHORT_THRESHOLD] = SINGLE_FAN_SHORT_CURRENT_THRESHOLD;
  holdingRegisters[REG_DF_SHORT_THRESHOLD] = DOUBLE_FAN_SHORT_CURRENT_THRESHOLD;

  holdingRegisters[REG_FAN_TYPE] = fanModelIndex;
  holdingRegisters[REG_FAN_PROFILE] = fanModeIndex;
  holdingRegisters[REG_Baud_Rate] = baudRateIndex;
  holdingRegisters[REG_Slave_id] = Slave_id;

  holdingRegisters[REG_VOLTAGE_LOW] = VOLTAGE_LOW_THRESHOLD;
  holdingRegisters[REG_VOLTAGE_HIGH] = VOLTAGE_HIGH_THRESHOLD;

  // vTaskDelay(10 / portTICK_PERIOD_MS);

  DEBUG_PRINTLN("Factory settings restored");
}

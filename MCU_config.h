#ifndef MCU_CONFIG_H
#define MCU_CONFIG_H

#include "Arduino.h"
#include "debug.h"
#include "debug_utils.h"

// Buttons Pin
#define BUTTON_UP   PB_11
#define BUTTON_DOWN PB_10
#define BUTTON_OK   PB_4
#define BUTTON_ESC  PB_5


#define DISPLAY_TYPE LCD
#define LCD_COL 16 
#define LCD_ROW 2 
// Define the pins for the 16x2 LCD
#define LCD_RS PB_2   // LCD Register select
#define LCD_EN PA_15  // LCD EN pin

// old prototype 
// #define LCD_D4 PB12
// #define LCD_D5 PB13
// #define LCD_D6 PB14
// #define LCD_D7 PB15


#define LCD_D4 PB_15
#define LCD_D5 PB_14
#define LCD_D6 PB_13
#define LCD_D7 PB_12

#define LCD_BL PB_0   // LCD Backlight Control Pin

// For 2.42" 128x64 OLED
// #define DISPLAY_TYPE OLED
// #define DISPLAY_COMM I2C
// #define DISPLAY_COMM SPI
#define LCD_CS   PB_15
#define LCD_SCK  PB_14
#define LCD_MISO PB_13
#define LCD_MOSI PB_12


// ODCC

// #define RELAY_1_Trigger  PB3   // Has LED indicator
// #define RELAY_2_Trigger  PA11
// #define RELAY_3_Trigger  PA1 
// #define RELAY_4_Trigger  PB6 
// #define RELAY_5_Trigger  PB7 
// #define RELAY_6_Trigger  PA8

#define RELAY_1_Trigger  PB_3   // Has LED indicator
#define RELAY_2_Trigger  PA_11
#define RELAY_3_Trigger  PA_1 
#define RELAY_4_Trigger  PB_6 
#define RELAY_5_Trigger  PB_7 
#define RELAY_6_Trigger  PA_8


#endif  // MCU_CONFIG_H

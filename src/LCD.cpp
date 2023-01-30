#include <Arduino.h>
#include <LiquidCrystal.h>

/// @brief LCD contrast, and pins for connecting the LCD to the arduino
int contrast = 50;
LiquidCrystal lcd(24, 22, 2, 3, 4, 5);

/// @brief Sets up the LCD with the contrast, rows and columns, sets the curser at pos 0,0.
void setupLCD()
{
    analogWrite(26, contrast);
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("Program start");
}

/// @brief First clears the row, then prints outputLCD on same row 
/// @param outputLCD The text to print
/// @param cursorPos The position of the curser
void setOutPutLCD(String outputLCD, int cursorPos)
{
    lcd.setCursor(0, cursorPos);
    lcd.print("                "); // Cleans the row
    lcd.setCursor(0, cursorPos);
    lcd.print(outputLCD);
}

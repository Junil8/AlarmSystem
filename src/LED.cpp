#include <Arduino.h>
/// Defining LED pins
#define Pin_LED_Red 11
#define Pin_LED_Green 12
#define Pin_LED_Blue 13

/// Setup Pins for LED
void setupPinLED()
{
    /// Setting LED pins to OUTPUT
    pinMode(Pin_LED_Red, OUTPUT);
    pinMode(Pin_LED_Green, OUTPUT);
    pinMode(Pin_LED_Blue, OUTPUT);
}

/// @brief Sets the output for the LEDs based on given parameter
/// @param red Sets the output for red. Valid number is between 0 - 255
/// @param green Sets the output for green. Valid number is between 0 - 255
/// @param blue Sets the output for blue. Valid number is between 0 - 255
void setOutputLED(int red, int green, int blue)
{
    analogWrite(Pin_LED_Red, red);
    analogWrite(Pin_LED_Green, green);
    analogWrite(Pin_LED_Blue, blue);
}
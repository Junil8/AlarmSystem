#include <Arduino.h>
#include <Keypad.h>

/// @brief Used for the keypads rows and columns
const byte ROWS = 4; // four rows
const byte COLS = 3; // three columns
char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

/// @brief The pins used when connecting the keypad to the arduino. I got help from this site with the setup https://www.ardumotive.com/how-to-use-a-keypad-en.html
byte rowPins[ROWS] = {46, 36, 38, 42};
byte colPins[COLS] = {44, 48, 40};

/// instance of the keypad class using the set row and col variables.
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
/// @brief returns the current pressed key
char keyPress()
{
    char key = keypad.getKey();
    return key;
}

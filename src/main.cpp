#include <Arduino.h>
#include <Keypad.h>
#include <Arduino_FreeRTOS.h>
#include "semphr.h"
#include <SPI.h>
#include <Ethernet.h>

/// defines buzzer pin
#define Pin_Buzzer 8
/// defines motion sensor pin
#define Pin_Motion_Sensor 9

/// @brief  mac for Webserver
byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
/// @brief Ipaddress for webserver
IPAddress ip(192, 168, 1, 177);
/// @brief Port for webserver
EthernetServer server(80);

/// prototype for setupPinLED
void setupPinLED();
/// prototype for setupLCD
void setupLCD();
/// prototype for TaskAlarm
void TaskAlarm(void *pvParameters);
/// prototype for TaskMotion
void TaskMotion(void *pvParameters);
/// prototype for TaskKey
void TaskKey(void *pvParameters);
/// prototype for TaskWebServer
void TaskWebServer(void *pvParameters);
/// prototype for displayController
void displayController();
/// prototype for inputController
void inputController();
/// prototype for buzzerSound
void buzzerSound(int pitch, int delay);
/// prototype for keyPress
char keyPress();
/// prototype for setOutPutLCD
void setOutPutLCD(String systemState, int cursorPos);
/// prototype for setOutputLED
void setOutputLED(int red, int green, int blue);
/// prototype for listenForClient
void listenForClient();

/// Used to check what state the system currently is in
enum systemStateEnum
{
  aktiveret,
  deaktiveret,
  alarm
};
/// @brief  Current state of the system
static int systemState = aktiveret;
/// @brief The code given by the user, used to change system state
static String userInput = "";
/// @brief A buffer for the motion sensor, so its not too sensitive
static int motionBuffer = 0; 
/// @brief to set state to deaktiveret write this code
static const String stateDisable = "123";
/// @brief to set state to aktiveret write this code
static const String stateEnable = "258";
/// @brief to set state to alarm write this code
static const String stopAlarm = "0";

/// @brief starts up webserver, Sets up pinmodes, Starts alarm, Key, Motion sensor and webserver tasks, starts the LCD and the LED light
void setup()
{
  /// @brief 
  Ethernet.begin(mac, ip);
  server.begin();

  Serial.begin(115200);
  pinMode(Pin_Motion_Sensor, INPUT);
  pinMode(Pin_Buzzer, OUTPUT);
  setupPinLED();
  setupLCD();

  xTaskCreate(TaskAlarm, "alarmTask", 100, NULL, 1, NULL);
  xTaskCreate(TaskKey, "keyTask", 100, NULL, 1, NULL);
  xTaskCreate(TaskMotion, "MotionTask", 100, NULL, 1, NULL);
  xTaskCreate(TaskWebServer, "WebServerTask", 400, NULL, 2, NULL);

  displayController();
}
/// @brief WebServer task, listen for clients connecting. 
void TaskWebServer(void *pvParameters __attribute__((unused)))
{
  while (true)
  {
    vTaskDelay(1);
    listenForClient();
  }
}
/// @brief  Alarm Task, plays a sound, if system state is in alarm
void TaskAlarm(void *pvParameters __attribute__((unused)))
{
  while (true)
  {
    vTaskDelay(1);
    if (systemState == alarm)
    {
      buzzerSound(250, 5);
      buzzerSound(500, 5);
      buzzerSound(750, 5);
      buzzerSound(500, 5);
    }
  }
}
/// @brief Motion sensor task, if motion is detected and the buffer is exceeded, change system state to alarm
void TaskMotion(void *pvParameters __attribute__((unused)))
{
  while (true)
  {
    vTaskDelay(1);
    bool motionDetected = digitalRead(Pin_Motion_Sensor);
    if (systemState == aktiveret)
    {
      if (motionDetected)
      {
        motionBuffer++;
      }
      else
      {
        motionBuffer--;
      }
      if (motionBuffer >= 100)
      {
        motionBuffer = 0;
        systemState = alarm;
        displayController();
      }
      else if (motionBuffer < 0)
      {
        motionBuffer = 0;
      }
    }
  }
}
/// @brief Keypad task, listens for keypresses, 0-9 adds to userinput, * deletes input, # compares userinput to relevant secret code.    
void TaskKey(void *pvParameters __attribute__((unused)))  
{
  while (true)
  {
    vTaskDelay(1);
    char key = keyPress();
    if (key)
    {
      if (key == '*')
      {
        userInput = "";
        setOutPutLCD(userInput, 1);
        if (systemState != alarm)
        {
          buzzerSound(200, 10);
        }
      }
      else if (key == '#')
      {
        inputController();
        displayController();
        if (systemState != alarm)
        {
          buzzerSound(650, 10);
        }
      }
      else
      {
        if (userInput.length() < 8)
        {
          userInput += key;
          setOutPutLCD(userInput, 1);
          if (systemState != alarm)
          {
            buzzerSound(key * 10, 10);
          }
        }
        else
        {
          if (systemState != alarm)
          {
            buzzerSound(100, 10);
          }
        }
      }
    }
  }
}
/// @brief Plays a sound via the buzzer component
/// @param pitch the pitch given in ints, eg 0-1000. 
/// @param delay the time between starting the tone and finishing it.
void buzzerSound(int pitch, int delay)
{
  tone(Pin_Buzzer, pitch);
  vTaskDelay(delay);
  noTone(Pin_Buzzer);
}
/// @brief Checks if the userinput is equal to any secret codes, and if so change the state acordingly
void inputController()
{
  if (userInput == stateDisable)
  {
    systemState = deaktiveret;
  }
  else if (userInput == stateEnable)
  {
    systemState = aktiveret;
  }
  else if (userInput == stopAlarm)
  {
    systemState = aktiveret;
  }
  userInput = "";
  setOutPutLCD(userInput, 1);
}
/// @brief Controls what the LCD and LED outputs
void displayController()
{
  switch (systemState)
  {
  /// case 0: Current state of system is "Aktiveret"
  case 0:
    setOutputLED(50, 50, 0);
    setOutPutLCD("Aktiveret", 0);
    break;
  /// case 1: Current state of system is "Deaktiveret"
  case 1:
    setOutputLED(0, 50, 0);
    setOutPutLCD("Deaktiveret", 0);
    break;
  /// case 2: Current state of system is "Alarm"
  case 2:
    setOutputLED(50, 0, 0);
    setOutPutLCD("Alarm", 0);
    break;
  /// default: If current state is invalid
  default:
    setOutputLED(0, 0, 50);
    setOutPutLCD("Invalid state", 0);
    break;
  }
}
/// @brief Listens for a client connecting to the webserver, and sends the website data to the client. 
void listenForClient()
{
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client)
  {
    // an HTTP request ends with a blank line
    bool currentLineIsBlank = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        Serial.print(c);

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the HTTP request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank)
        {
          // send a standard HTTP response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close"); // the connection will be closed after completion of the response
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head>");
          client.println("<style>");
          client.println("* {");
          client.println("margin: 0;");
          client.println("padding: 0;");
          client.println("}");
          client.println("</style>");
          client.println("</head>");
          client.println("<body style='background-color: whitesmoke'>");
          client.println("<div style='padding: 10px; background-color: mediumseagreen; text-align: center;'>");
          client.println("<h1>GenJen</h1>");
          client.println("<h1>Security System</h1>");
          client.println("</div>");
          client.println("<div style='width: 50%; margin: auto; text-align: center;'>");
          client.println("<br><br>");
          client.println("<h2 style='display: inline;'> Aktuel tilstand: </h2>");
          client.println("<h2 style=' display: inline;'> Aktiveret </h2>");
          client.println("<br><br>");
          client.println("<form method='POST' action='/test'>");
          client.println("<label for='tilstand'>Vælg tilstand</label>");
          client.println("<select name='tilstand'>");
          client.println("<option value='0'>Aktiveret</option>");
          client.println("<option value='1'>Deaktiveret</option>");
          client.println("<option value='2'>Alarm</option>");
          client.println("</select>");
          client.println("<input type='submit' value='Submit'>");
          client.println("</form> ");
          client.println("<br><br>");
          client.println("<table style='border: 1px solid; font-size: xx-large; margin: auto;'>");
          client.println("<tr>");
          client.println("<th colspan='3'>Alameringer</th>");
          client.println("</tr>");
          client.println("<tr>");
          client.println("<td style='padding-right:10px;'>Alarm</td>");
          client.println("<td style='padding-right:10px;'>10:00</td>");
          client.println("<td style='padding-right:10px;'>27/1/2023</td>");
          client.println("</tr>");
          client.println("</table>");
          client.println("</div>");
          client.println("</body>");
          client.println("</html>");
          break;
        }
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // close the connection:
    client.stop();
  }
}
void loop() {}
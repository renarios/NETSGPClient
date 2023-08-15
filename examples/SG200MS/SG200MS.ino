#include "NETSGPClient.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

constexpr const uint8_t PROG_PIN = 4; /// Programming enable pin of RF module
constexpr const uint8_t RX_PIN = 16; /// RX pin of ESP32 connect to TX of RF module
constexpr const uint8_t TX_PIN = 17; /// TX pin of ESP32 connect to RX of RF module
constexpr const uint32_t inverterID = 0x11002945; /// Identifier of your inverter (see label on inverter)
//constexpr const uint32_t inverterID = 0x1100279; /// Identifier of your inverter (see label on inverter)
//constexpr const uint32_t inverterID = 11000001; /// Identifier of your inverter (see label on inverter)

#if defined(ESP32)
// On ESP32 debug output is on Serial and RF module connects to Serial2
#define debugSerial Serial
#define clientSerial Serial2
#else
// On ESP8266 or other debug output is on Serial1 and RF module connects to Serial
// On D1 Mini connect RF module to pins marked RX and TX and use D4 for debug output
#define debugSerial Serial1
#define clientSerial Serial
#endif

NETSGPClient client(clientSerial, PROG_PIN); /// NETSGPClient instance

void setup()
{
    // Initialyse SSD1306 LCD-screen via I2C
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        debugSerial.println(F("SSD1306 initialisatiefout"));
        while(true);
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    debugSerial.begin(115200);
#if defined(ESP32)
    clientSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
#else
    clientSerial.begin(9600);
#endif
    pinMode(LED_BUILTIN, OUTPUT);
    delay(1000);
    debugSerial.println("Welcome to Micro Inverter Interface by ATCnetz.de and enwi.one");

    // Make sure the RF module is set to the correct settings
    if (!client.setDefaultRFSettings())
    {
        debugSerial.println("Could not set RF module to default settings");
        // Clear display before writing new values
        display.clearDisplay();
        // Update the screen values
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println("Could not set RF module to default settings");
        // Print the updates values on the OLED screen
        display.display();
        delay(5000);
    }
}

uint32_t lastSendMillis = 0;
void loop()
{
    const uint32_t currentMillis = millis();
    if (currentMillis - lastSendMillis > 4000)
    {
        // Print the values on the OLED screen
        display.setTextSize(1);
        display.setCursor(0, 0);

        lastSendMillis = currentMillis;

        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        debugSerial.println("");
        debugSerial.println("Sending request now");
        // Clear display before writing new values
        //display.clearDisplay();
        //display.println("Sending request now");

        const NETSGPClient::InverterStatus status = client.getStatus(inverterID);
        if (status.valid)
        {
          debugSerial.println("*********************************************");
          debugSerial.println("Received Inverter Status");
          debugSerial.print("Device: ");
          debugSerial.println(status.deviceID, HEX);
          debugSerial.println("Status: " + String(status.state));
          debugSerial.println("DC_Voltage: " + String(status.dcVoltage) + "V");
          debugSerial.println("DC_Current: " + String(status.dcCurrent) + "A");
          debugSerial.println("DC_Power: " + String(status.dcPower) + "W");
          debugSerial.println("AC_Voltage: " + String(status.acVoltage) + "V");
          debugSerial.println("AC_Current: " + String(status.acCurrent) + "A");
          debugSerial.println("AC_Power: " + String(status.acPower) + "W");
          debugSerial.println("Power gen total: " + String(status.totalGeneratedPower));
          debugSerial.println("Temperature: " + String(status.temperature));

        // Wis het vorige scherm voordat je nieuwe gegevens weergeeft
        display.clearDisplay();

        // Update the screen values
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println("AC_Power:    " + String(status.acPower) + " W");
        display.println("Power total: " + String(status.totalGeneratedPower) + " KW");
        display.println("Temperature: " + String(status.temperature) +  " C");
        display.println("AC_Voltage:  " + String(status.acVoltage) + " V");
        display.println("AC_Current:  " + String(status.acCurrent) + " A");
        display.println("DC_Voltage:  " + String(status.dcVoltage) + " V");
        display.println("DC_Current:  " + String(status.dcCurrent) + " A");
        display.println("DC_Power:    " + String(status.dcPower) + " W");
        }

        // Print the updates values on the OLED screen
        display.display();
    }
}

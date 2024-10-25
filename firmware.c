#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>

// Pin definitions
#define TFT_CS     10
#define TFT_DC     9
#define TFT_RST    8
#define SD_CS      4
#define BUZZER_PIN 7
#define LED_INDICATOR_LEFT 6
#define LED_INDICATOR_RIGHT 5
#define SWITCH_LEFT 2
#define SWITCH_RIGHT 3
#define POT_PIN A0

//display object
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
RTC_DS3231 rtc;

//status information
int speed = 0;
bool leftIndicator = false;
bool rightIndicator = false;
bool keyStatus = true; 
int batteryPercentage = 100; 
bool sideStandStatus = false; 
const char* driverMode = "Normal";

// Time interval constants
const unsigned long saveInterval = 60000; // Save data every minute

// Debounce timing variables
unsigned long lastDebounceTimeLeft = 0;
unsigned long lastDebounceTimeRight = 0;
const unsigned long debounceDelay = 500; // Debounce delay in milliseconds

void setup() {
    Serial.begin(9600);
    
    // Initialize display
    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(ILI9341_BLACK);
    
    // Initialize SD card
    if (!SD.begin(SD_CS)) {
        Serial.println("SD card initialization failed!");
        return;
    }
    
    // Initialize RTC
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }

    // Set up pins
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_INDICATOR_LEFT, OUTPUT);
    pinMode(LED_INDICATOR_RIGHT, OUTPUT);
    pinMode(SWITCH_LEFT, INPUT_PULLUP);
    pinMode(SWITCH_RIGHT, INPUT_PULLUP);

    //Display initial information
    displayInfo();
}

void loop() {
    static unsigned long lastSaveTime = 0;

    //potentiometer for speed simulation
    int potValue = analogRead(POT_PIN);
    speed = map(potValue, 0, 1023, 0, 120); // Map to speed range

    //Update display with current info only if necessary
    displayInfo();

    //switch states for indicators with debounce 
    handleIndicatorSwitches();

    //Save data to SD card every minute (60000 ms)
    if (millis() - lastSaveTime >= saveInterval) {
        saveDataToSD();
        lastSaveTime = millis();
    }

    delay(100); //Loop delay for stability
}

void handleIndicatorSwitches() {
    if (digitalRead(SWITCH_LEFT) == LOW && millis() - lastDebounceTimeLeft > debounceDelay) {
        leftIndicator = !leftIndicator; 
        digitalWrite(LED_INDICATOR_LEFT, leftIndicator ? HIGH : LOW);
        soundBuzzer();
        lastDebounceTimeLeft = millis();
    }
    
    if (digitalRead(SWITCH_RIGHT) == LOW && millis() - lastDebounceTimeRight > debounceDelay) {
        rightIndicator = !rightIndicator;
        digitalWrite(LED_INDICATOR_RIGHT, rightIndicator ? HIGH : LOW);
        soundBuzzer();
        lastDebounceTimeRight = millis();
    }
}

void displayInfo() {
    tft.fillScreen(ILI9341_BLACK);
    
    DateTime now = rtc.now();
    
    tft.setTextColor(ILI9341_WHITE);
    
    tft.setCursor(10, 10);
    tft.setTextSize(2);
    
    tft.print("Speed: ");
    tft.print(speed);

    tft.setCursor(10, 40);
    tft.print("Indicators: ");
    tft.print(leftIndicator ? "Left ON " : "Left OFF ");
    tft.print(rightIndicator ? "Right ON" : "Right OFF");

    tft.setCursor(10, 70);
    tft.print("Key Status: ");
    tft.print(keyStatus ? "ON" : "OFF");

    tft.setCursor(10, 100);
    tft.print("Battery: ");
    tft.print(batteryPercentage);

    tft.setCursor(10, 130);
    tft.print("Side Stand: ");
    tft.print(sideStandStatus ? "Down" : "Up");

    tft.setCursor(10, 160);
    tft.print("Mode: ");
    tft.print(driverMode);

   //Format time display properly 
   String formattedTime = String(now.hour()) + ":" + (now.minute() < 10 ? "0" : "") + String(now.minute());
   tft.setCursor(10, 190);
   tft.print("Time: ");
   tft.print(formattedTime);
}

void soundBuzzer() {
   tone(BUZZER_PIN, 1000); 
   delay(100); 
   noTone(BUZZER_PIN);
}

void saveDataToSD() {
   File dataFile = SD.open("datalog.txt", FILE_WRITE);
   if (dataFile) {
       DateTime now = rtc.now();
       dataFile.print("Speed: ");
       dataFile.print(speed);
       dataFile.print(", Time: ");
       dataFile.print(now.hour());
       dataFile.print(":");
       dataFile.print(now.minute() < 10 ? "0" : "");
       dataFile.print(now.minute());
       dataFile.println();
       dataFile.close();
   } else {
       Serial.println("Error opening datalog.txt");
   }
}
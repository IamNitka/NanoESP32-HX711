#include "HX711.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_sleep.h"

// Number of scales
const byte numScales = 6;

// Pins for data and clock of each scale
byte dataPins[numScales] = {2, 3, 4, 5, 6, 7};
byte clockPins[numScales] = {8, 9, 10, 11, 12, 13};

// Calibration values
float cal[numScales] = {435.17, 400, 233, 333, 232, 432};

// Array for each HX711 instance
HX711 scales[numScales];

// WiFi credentials
const char* ssid = "CRM_BaileyStreet";
const char* password = "39baileystreet";

// URL for sending data to Google Sheets
const String scriptUrl = "https://script.google.com/macros/s/AKfycbwzGHQkW-zkDKtslXPBnZMrGIIc75dHhi1HtNoO1XrGBkQh2syfrs2dP7AAFtY7xbR7/exec?";

// Deep sleep duration (microseconds) - 3 minutes
const uint64_t uS_TO_S_FACTOR = 1000000ULL;
const uint64_t TIME_TO_SLEEP = 3 * 60 * uS_TO_S_FACTOR;

// RTC memory to store tare offsets
RTC_DATA_ATTR float tareOffsets[numScales];
RTC_DATA_ATTR bool isTareSet = false;

// LED Pins
const int redLED = 46;
const int greenLED = 0;
const int blueLED = 45;

void setup() {
  Serial.begin(9600);

  // Set pinMode for LEDs
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);

  // Start with all LEDs off
  digitalWrite(redLED, HIGH);
  digitalWrite(greenLED, HIGH);
  digitalWrite(blueLED, HIGH);

  // Connecting to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(blueLED, LOW); // Turn on blue LED if unable to connect to WiFi
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  digitalWrite(blueLED, HIGH); // Turn off blue LED when connected to WiFi
  Serial.println("Connected to WiFi");

  for (byte i = 0; i < numScales; i++) {
    scales[i].begin(dataPins[i], clockPins[i]);
    delay(2000);

    scales[i].set_scale(cal[i]);
    delay(2000);
  }
  Serial.println("Scale Calibrated.");

  if (!isTareSet) {
    // Set tare offsets if they are not set
    for (byte i = 0; i < numScales; i++) {
      scales[i].tare();
      tareOffsets[i] = scales[i].get_offset();
    }
    isTareSet = true;
    Serial.println("Scale Tared and offsets saved.");
  } else {
    // Use stored tare offsets
    for (byte i = 0; i < numScales; i++) {
      scales[i].set_offset(tareOffsets[i]);
    }
    Serial.println("Scale Tare offsets restored.");
  }

  digitalWrite(greenLED, LOW); // Turn on green LED when starting normally
  delay(1000); // Delay to make the green LED visible
  digitalWrite(greenLED, HIGH);
}

void loop() {
  Serial.println("Reading scales:");

  // Looping for sending the data; j = loop times
  for (byte j = 0; j < 10; j++) {
    digitalWrite(greenLED, LOW); // Turn on green LED while reading from scales
    String data = "";
    for (byte i = 0; i < numScales; i++) {
      if (scales[i].is_ready()) {
        float reading = scales[i].get_units(10);
        Serial.print("Scale ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(reading);

        // Prepare the sending data
        if (i > 0) {
          data += "&";
        }
        data += "value" + String(i + 1) + "=" + String(reading);

      } else {
        Serial.print("Scale ");
        Serial.print(i);
        Serial.println(" not ready");
      }
    }

    // Send the data to Google Sheet
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String fullUrl = scriptUrl + data;
      http.begin(fullUrl);
      int httpResponseCode = http.GET();
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
      } else {
        Serial.print("Error on sending GET: ");
        Serial.println(httpResponseCode);
        digitalWrite(greenLED, HIGH);
        digitalWrite(redLED, LOW); // Turn on red LED when unable to upload data
        delay(1000); // Delay to make the red LED visible
        digitalWrite(redLED, HIGH);
      }
      http.end();
    }
    digitalWrite(greenLED, HIGH); // Turn off green LED after reading from scales
    delay(2000); // Delay 2 seconds between each data send
  }

  // Set deep sleep
  Serial.println("Entering deep sleep for 3 minutes...");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP);
  esp_deep_sleep_start();
}

## NanoESP32 for Multiple-scales via HX711

The code was setup on Arduino NanoESP32 connected with 6 sets of HX711 (M5-stack) and loadcell. The scales will read to read 10 values and send the data to scriptUrl (GoogleSheet), then deep sleep for 3 mins.

### Hardware setup

**Load Cells and HX711 Connection**

Connect each HX711 amplifier to the Arduino NanoESP32 using the following GPIO pins:

Data Pins: GPIO 2, 3, 4, 5, 6, 7
Clock Pins: GPIO 8, 9, 10, 11, 12, 13

Connect the load cells to the HX711 amplifiers as per their respective pin configurations.

**Build-in LED Indicators**

1. Red LED: error indication (ON if there is an error uploading data)
2. Green LED: normal operation
3. Blue LED: indicate WiFi connection (ON if the device cannot connect ot the WiFi)

### Software setup

The code requires 4 libraries
1. HX711.h by Rob Tillaart v0.4.0
2. WiFi.h 
3. HTTPClient.h
4. esp_sleep.h

**Google script**
The result will be upload to the Google Sheet by set up the *'scriptUrl'*, the AppsScript in Google sheet is required

**WiFi Credential**
Update the *'ssid'* and *'password'* variables with your WiFi network

**Calibration value**
Ensure the calibration values for each load cell are correctly set in the *'cal'* array.


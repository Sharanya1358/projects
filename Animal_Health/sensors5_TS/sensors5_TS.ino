#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

#define REPORTING_PERIOD_MS 1000

// ThingSpeak settings
const char* ssid = "Redmi Note 9";           // Your WiFi SSID
const char* password = "1234567890";           // Your WiFi Password
unsigned long myChannelNumber = 2779187;     // Your ThingSpeak channel ID
const char* myWriteAPIKey = "N6VSSSX3TFNS4MJB"; // Your ThingSpeak Write API Key

// MAX30100 Heart Rate Sensor Configuration
PulseOximeter pox;
float BPM, SpO2;
uint32_t tsLastReport = 0;

// DS18B20 Temperature Sensor Configuration
#define DS18B20 4  // DS18B20 data pin connected to GPIO2
OneWire ourWire(DS18B20);
DallasTemperature sensor(&ourWire);

// Shared analog input (A0)
#define XPin A0  // ADXL335 or GSR sensor on A0
float xVoltage;
int gsrValue;
bool readGSR = false; // Toggle between ADXL335 and GSR readings

// Create a WiFiClient object
WiFiClient client;

// Callback for MAX30100 beat detection
void onBeatDetected() {
    Serial.println("Beat Detected!");
}

// Function to read and print temperature data
void sendTemperature() {
    sensor.requestTemperatures();
    float tempC = sensor.getTempCByIndex(0);
    if (tempC == DEVICE_DISCONNECTED_C) {
        Serial.println("Error: DS18B20 sensor not connected!");
        return;
    }
    Serial.print("Celsius: ");
    Serial.println(tempC);
    ThingSpeak.setField(1, tempC);  // Field 1: Temperature (Celsius)
}

// Function to read and send ADXL335 X-axis voltage or GSR value
void sendAnalogSensorData() {
    if (readGSR) {
        // Read GSR sensor value
        gsrValue = analogRead(XPin);
        Serial.print("GSR Value: ");
        Serial.println(gsrValue);
        ThingSpeak.setField(5, gsrValue);  // Field 5: GSR Value
    } else {
        // Read ADXL335 X-axis voltage
        int xValue = analogRead(XPin);
        xVoltage = (xValue / 1023.0) * 3.3; // Convert raw ADC value to voltage
        Serial.print("X-Axis Voltage: ");
        Serial.println(xVoltage);
        ThingSpeak.setField(4, xVoltage);  // Field 4: ADXL335 X-axis voltage
    }
    // Toggle between sensors for the next reading
    readGSR = !readGSR;
}

void setup() {
    Serial.begin(115200);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi Connected!");

    // Initialize ThingSpeak
    ThingSpeak.begin(client);

    // Initialize Pulse Oximeter (MAX30100)
    Serial.print("Initializing Pulse Oximeter...");
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
        pox.setOnBeatDetectedCallback(onBeatDetected);
    }
    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

    // Initialize DS18B20 sensor
    sensor.begin();
}

void loop() {
    // Update MAX30100 data
    pox.update();

    // Read and send temperature data
    sendTemperature();

    // Alternate between ADXL335 and GSR sensor data
    sendAnalogSensorData();

    // Read and send heart rate and SpO2 data
    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart rate: ");
        Serial.print(BPM);
        Serial.print(" bpm / SpO2: ");
        Serial.print(SpO2);
        Serial.println(" %");

        ThingSpeak.setField(2, BPM);  // Field 2: Heart Rate
        ThingSpeak.setField(3, SpO2); // Field 3: SpO2

        // Write all fields to ThingSpeak
        ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

        tsLastReport = millis();
    }
    delay(1000);  // Delay between updates
}

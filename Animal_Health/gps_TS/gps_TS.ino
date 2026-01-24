#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// ThingSpeak credentials
const char* ssid = "Redmi Note 9";         // Replace with your WiFi SSID
const char* password = "sharanya"; // Replace with your WiFi password
const char* server = "api.thingspeak.com"; // ThingSpeak API server
const char* apiKey = "XWA0I6LUXLQ82JZA";    // Replace with your ThingSpeak Write API Key

TinyGPSPlus gps;          // Create TinyGPS++ object
SoftwareSerial ss(4, 5);  // RX = GPIO4 (D2), TX = GPIO5 (D1)
WiFiClient client;        // WiFi client for ThingSpeak

void setup() {
  Serial.begin(115200);   // Initialize Serial Monitor
  ss.begin(9600);         // Initialize GPS baud rate
  Serial.println("GPS Module is starting...");

  // Connect to WiFi
  connectWiFi();
}

void loop() {
  // Read data from the GPS module
  while (ss.available() > 0) {
    if (gps.encode(ss.read())) {
      sendToThingSpeak();  // Send parsed GPS data to ThingSpeak
    }
  }

  // Check if the GPS is not receiving valid data
  if (gps.charsProcessed() < 10) {
    Serial.println("No GPS data received: Check wiring or GPS reception.");
    delay(2000);
  }
}

void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void sendToThingSpeak() {
  if (gps.location.isValid()) {
    // GPS Data
    double latitude = gps.location.lat();
    double longitude = gps.location.lng();
    double altitude = gps.altitude.isValid() ? gps.altitude.meters() : 0.0;
    double speed = gps.speed.isValid() ? gps.speed.kmph() : 0.0;

    // Print GPS Data to Serial Monitor
    Serial.print("Latitude: ");
    Serial.println(latitude, 6);
    Serial.print("Longitude: ");
    Serial.println(longitude, 6);
    Serial.print("Altitude (meters): ");
    Serial.println(altitude);
    Serial.print("Speed (km/h): ");
    Serial.println(speed);

    // Send Data to ThingSpeak
    if (client.connect(server, 80)) {
      String url = "/update?api_key=";
      url += apiKey;
      url += "&field1=" + String(latitude, 6);
      url += "&field2=" + String(longitude, 6);
      url += "&field3=" + String(altitude);
      url += "&field4=" + String(speed);

      Serial.print("Connecting to ThingSpeak... Sending data: ");
      Serial.println(url);

      // HTTP GET Request
      client.print("GET " + url + " HTTP/1.1\r\n");
      client.print("Host: " + String(server) + "\r\n");
      client.print("Connection: close\r\n\r\n");

      unsigned long timeout = millis();
      while (client.available() == 0) {
        if (millis() - timeout > 5000) {
          Serial.println(">>> Client Timeout!");
          client.stop();
          return;
        }
      }

      // Print the response from ThingSpeak
      Serial.println("ThingSpeak Response:");
      while (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.println(line);
      }

      client.stop();
      Serial.println("Data sent to ThingSpeak successfully.");
    } else {
      Serial.println("Failed to connect to ThingSpeak.");
    }
  } else {
    Serial.println("Location not valid, data not sent.");
  }

  Serial.println();  // Blank line between updates
  delay(20000);      // ThingSpeak allows updates every 15 seconds
}

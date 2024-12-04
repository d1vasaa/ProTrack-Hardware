#include <WiFi.h>
#include <HTTPClient.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

#define GPS_RX_PIN 16 // GPS TX pin connected to Arduino RX pin
#define GPS_TX_PIN 17 // GPS RX pin connected to Arduino TX pin

SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN); // Create a software serial port for GPS module
TinyGPSPlus gps; // Create a TinyGPS++ object

// const char* ssid = "pocopoco2@unifi"; // Replace with your WiFi network SSID
// const char* password = "24681012"; // Replace with your WiFi network password
const char* ssid = "Aimanisaac"; // Replace with your WiFi network SSID
const char* password = "rasydan808"; // Replace with your WiFi network password
const char* serverAddress = "https://protrack.pagekite.me"; // Replace with your server address
const int serverPort = 80;  // HTTP default port

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(115200); // Initialize serial communication with the GPS module
  delay(10);

  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Print the IP address
  Serial.println(WiFi.localIP());
  
  // Send a message indicating waiting for GPS data
  Serial.println("Waiting for GPS data...");
}

void loop() {
  // Read data from GPS module
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        // Print latitude and longitude to the Serial Monitor
        Serial.print("Latitude: ");
        Serial.print(gps.location.lat(), 6);
        Serial.print(", Longitude: ");
        Serial.println(gps.location.lng(), 6);
        sendToServer(gps.location.lat, gps.location.lng);
      }
    }
  }
  
  // Check if there's no valid GPS data
  if (!gps.location.isValid()) {
    // Send an error message
    Serial.println("Error: No valid GPS data received.");
    // You can add additional error handling code here if needed
  }

  delay(1000);
}

void sendToServer(float value, float value2) {
  WiFiClient client;

  if (client.connect(serverAddress, serverPort)) {  // Connect to the server with the specified port
    Serial.println("Connected to server");
    HTTPClient http;

    // Create the POST request body
    String postData = "value=" + String(value, 7) + "&value2=" + String(value2, 7); // Convert float to string with 7 decimal places

    // Send the HTTP POST request
    http.begin(client, serverAddress, serverPort, "/index.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpCode = http.POST(postData);

    if (httpCode > 0) {
      Serial.printf("[HTTP] POST request code: %d\n", httpCode);

      // Check HTTP status code
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
        String payload =  http.getString();
        Serial.println(postData);        Serial.println("Server response: " + payload);
      }
    } else {
      Serial.printf("[HTTP] POST request failed: %s\n", http.errorToString(httpCode).c_str());
    }

    // Disconnect
    http.end();
    client.stop();
    Serial.println("Connection closed");
  } else {
    Serial.println("Failed to connect to server");
  }
}

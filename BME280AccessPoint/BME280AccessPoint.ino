/***********************************
  Weather Access Point using NodeMCU and BME280:
  - temperature
  - humidity
  - pressure
  
  Implements Adafruit's sensor libraries:
  - Adafruit_Sensor
  - Adafruit_BME280
  
 *  HOW TO USE:
 *  connect to the ESP8266 AP then
 *  use web broswer to go to 192.168.4.1
 *  and you will see: temperature, humidity and pressure.
 *  
 *  CONNECTIONS:
 *  
 *  NodeMCU -> BME280
 *  3.3V - VIN
 *  GND  - GND
 *  D3   - SDA
 *  D4   - SCL
***********************************/

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h> 

Adafruit_BME280 bme; // I2C

// Replace with your network details
const char* ssid = "ESP_BME280_AP"; // WiFi-ssid
const char* password = "followthesun"; //WiFi-password
float humidity, temperature_C,temperature_F, pressure, pin, dp;
char temperatureCString[6];
char dpString[6];
char humidityString[6];
char pressureString[7];
char pressureInchString[6];

// Web Server on port 80
WiFiServer server(80);

// only runs once on boot
void setup() {
  // Initializing serial port for debugging purposes
  Serial.begin(115200);
  delay(10);
  Wire.begin(D3, D4);
  Wire.setClock(100000);
  // Connecting to WiFi network
  Serial.println();
  Serial.print("Configuring access point...");
  Serial.println(ssid);
  
  boolean conn = WiFi.softAP(ssid, password); 
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  //server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

void getWeather() {
  
    humidity = bme.readHumidity();
    temperature_C = bme.readTemperature();
    temperature_F = temperature_C*1.8+32.0;
    dp = temperature_F-0.36*(100.0-humidity);  
    pressure = bme.readPressure()/100.0F; 
    pin = 0.02953*pressure;
    
    dtostrf(temperature_C, 5, 1, temperatureCString);
    dtostrf(humidity, 5, 1, humidityString);
    dtostrf(pressure, 6, 1, pressureString);
    dtostrf(pin, 5, 2, pressureInchString);
    dtostrf(dp, 5, 1, dpString);
    delay(100);
}

// runs over and over again
void loop() {
  // Listenning for new clients
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New client");
    // bolean to locate when the http request ends
    boolean blank_line = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if (c == '\n' && blank_line) {
            getWeather();
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            // your actual web page that displays temperature
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head><META HTTP-EQUIV=\"refresh\" CONTENT=\"15\"></head>");
            client.println("<body><h1>ESP8266 Weather Access Point</h1>");
            client.println("<table border=\"2\" width=\"456\" cellpadding=\"10\"><tbody><tr><td>");
            client.println("<h3>Temperature = ");
            client.println(temperatureCString);
            client.println("&deg;C</h3><h3>Humidity = ");
            client.println(humidityString);           
            client.println("%</h3><h3>Pressure = ");        
            client.println(pressureString);
            client.println("hPa (");
            client.println(pressureInchString);
            client.println("Inch)</h3></td></tr></tbody></table></body></html>");  
            break;
        }
        if (c == '\n') {
          // when starts reading a new line
          blank_line = true;
        }
        else if (c != '\r') {
          // when finds a character on the current line
          blank_line = false;
        }
      }
    }  
    // closing the client connection
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
} 

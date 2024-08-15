#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const char* ssid = "TP-Link_1154";
const char* password = "88814766";

// Create an instance of the server
ESP8266WebServer server(80);

// Voltage sensor pin
const int sensorPin = A0;

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// uv led 
#define vu_led 8

// Store the HTML content in program memory
const char webpage[] PROGMEM = R"=====(

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Temperature Control</title>
    <style>
        body {
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
        }

        .container {
            width: 300px;
            background-color: #fff;
            border-radius: 20px;
            box-shadow: 0 0 20px rgba(0, 0, 0, 0.1);
            text-align: center;
            padding: 20px;
        }

        .header {
            display: flex;
            justify-content: center;
            align-items: center;
            position: relative;
        }

        .header h2 {
            margin: 0;
            font-size: 18px;
        }

        .temp-display {
            margin: 20px 0;
            position: relative;
        }

        .circle {
            width: 150px;
            height: 150px;
            border-radius: 50%;
            background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
            display: flex;
            justify-content: center;
            align-items: center;
            margin: 0 auto;
            position: relative;
        }

        .temp {
            font-size: 48px;
            color: #fff;
            position: absolute;
        }

        .temp-info {
            display: flex;
            justify-content: space-around;
        }

        .temp-info .value {
            font-size: 18px;
            color: #333;
        }

        .box {
            margin: 0 8px; 
            padding: 18px 10px;
            border-radius: 12px;
            background-image: -o-linear-gradient(135deg, #f0f1fa, #f7f8fd);
            box-shadow: 4px 4px 8px rgba(0,0,0,0.08), -4px -4px 4px #ffffff;
            text-align: center;
            border: 2px solid #4CAF50;
        }

        .button-link {
            display: inline-block;
            padding: 10px 20px;
            border-radius: 12px;
            background-color: #ffffff;
            color: rgb(0, 0, 0);
            text-decoration: none;
            font-size: 20px;
            transition: background-color 0.3s;
        }

        .fr {
            margin: 0 8px; 
            padding: 18px 10px;
            border-radius: 12px;
            background-image: -o-linear-gradient(135deg, #f0f1fa, #f7f8fd);
            box-shadow: 4px 4px 8px rgba(0,0,0,0.08), -4px -4px 4px #ffffff;
            text-align: center;
        }

        .led-controls {
            margin-top: 20px;
        }

        .led-controls button {
            padding: 10px 20px;
            font-size: 16px;
            margin: 5px;
            border-radius: 12px;
            border: none;
            cursor: pointer;
            color: #fff;
        }

        .led-on {
            background-color: #4CAF50;
        }

        .led-off {
            background-color: #f44336;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h2>iot fridge</h2>
        </div>
 
        <div class="temp-display">
            <div class="circle">
                <span class="temp" id="tempValue"><p><strong>Loading...</strong></p></span>
            </div>
        </div>
        
        <div class="temp-info">
            <div class="fr">Voltage<br><span class="value" id="voltageValue">Loading...</span></div>
        </div>

        <div class="led-controls">
            <button class="led-on" onclick="toggleLED('on')">Turn On UV LED</button>
            <button class="led-off" onclick="toggleLED('off')">Turn Off UV LED</button>
        </div>
    </div>

    <script>
        // Function to fetch temperature and voltage data from the server
        function fetchData() {
            fetch('/temperature')
            .then(response => response.json())
            .then(data => {
                document.getElementById('tempValue').innerHTML = "<p><strong>" + data.temperature + " &deg;C</strong></p>";
                document.getElementById('voltageValue').innerText = data.voltage + "V";
            })
            .catch(error => console.error('Error:', error));
        }

        // Function to toggle LED state
        function toggleLED(state) {
            fetch('/led?state=' + state)
            .then(response => {
                if (response.ok) {
                    alert('UV LED turned ' + state);
                } else {
                    alert('Failed to toggle LED');
                }
            })
            .catch(error => console.error('Error:', error));
        }

        // Refresh data every 2 seconds
        setInterval(fetchData, 2000);

        // Initial fetch
        fetchData();
    </script>
</body>
</html>


)=====";

void handleRoot() {
  server.send(200, "text/html", webpage);
}

void handleTemperature() {
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  
  int sensorValue = analogRead(sensorPin);
  float voltage = sensorValue * (3.3 / 1023.0);

  String data = "{\"temperature\":" + String(temperatureC) + ", \"voltage\":" + String(voltage) + "}";
  server.send(200, "application/json", data);
}
void handleLED() {
  String state = server.arg("state");
  if (state == "on") {
    digitalWrite(vu_led, HIGH);
    server.send(200, "text/plain", "LED is ON");
  } else if (state == "off") {
    digitalWrite(vu_led, LOW);
    server.send(200, "text/plain", "LED is OFF");
  } else {
    server.send(400, "text/plain", "Invalid State");
  }
}


void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Set LED pin mode
  pinMode(vu_led, OUTPUT);
  digitalWrite(vu_led, LOW); // Start with the LED off

  // Define the root handler
  server.on("/", handleRoot);

  // Define the temperature handler
  server.on("/temperature", handleTemperature);

  // Define the LED control handler
  server.on("/led", handleLED);

  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  delay(500);
}

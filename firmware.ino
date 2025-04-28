#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>

// WiFi Access Point configuration
const char *ssid = "DistanceMonitorAP"; // Hotspot name
const char *password = "12345678";    // Hotspot password

// HC-SR04 pins
const int trigPin = D1;  // GPIO5
const int echoPin = D2;  // GPIO4

// WebSocket server on port 81 and HTTP server on port 80
WebSocketsServer webSocket(81);
ESP8266WebServer server(80);

// HTML page with embedded CSS and JavaScript
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP8266 Distance Monitor</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 0;
            padding: 20px;
            background-color: #f5f5f5;
        }
        .container {
            max-width: 500px;
            margin: 0 auto;
            background-color: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        #distance {
            font-size: 48px;
            font-weight: bold;
            margin: 20px;
            color: #2c3e50;
        }
        .unit-toggle {
            margin: 20px;
        }
        button {
            background-color: #3498db;
            color: white;
            border: none;
            padding: 10px 20px;
            margin: 5px;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
        }
        button:hover {
            background-color: #2980b9;
        }
        .status {
            margin-top: 20px;
            color: #7f8c8d;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Ultrasonic Distance Monitor</h1>
        <div id="distance">--</div>
        <div class="unit-toggle">
            <button onclick="setUnit('cm')">Centimeters</button>
            <button onclick="setUnit('in')">Inches</button>
        </div>
        <div class="status">
            <p>Connected to: DistanceMonitorAP</p>
            <p id="ip-address"></p>
        </div>
    </div>
    
    <script>
        let unit = 'cm';
        let websocket;
        
        function connectWebSocket() {
            websocket = new WebSocket('ws://' + window.location.hostname + ':81/');
            
            websocket.onopen = function() {
                console.log("WebSocket connected");
                document.getElementById('ip-address').textContent = 
                    "IP: " + window.location.hostname;
            };
            
            websocket.onmessage = function(event) {
                const data = JSON.parse(event.data);
                const distance = unit === 'cm' ? data.distance : (data.distance / 2.54);
                document.getElementById('distance').innerHTML = 
                    distance.toFixed(2) + ' ' + (unit === 'cm' ? 'cm' : 'in');
            };
            
            websocket.onclose = function() {
                console.log("WebSocket disconnected");
                setTimeout(connectWebSocket, 1000); // Reconnect after 1 second
            };
        }
        
        function setUnit(newUnit) {
            unit = newUnit;
            if (websocket && websocket.readyState === WebSocket.OPEN) {
                websocket.send(unit);
            }
        }
        
        // Initialize connection when page loads
        window.onload = function() {
            connectWebSocket();
        };
    </script>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Initialize HC-SR04 pins
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    
    // Create Access Point
    Serial.println();
    Serial.println("Creating Access Point...");
    WiFi.softAP(ssid, password);
    
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    
    // Start WebSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket server started");
    
    // Handle HTTP requests
    server.on("/", []() {
        server.send(200, "text/html", htmlPage);
    });
    
    server.begin();
    Serial.println("HTTP server started");
    
    Serial.println("System ready. Connect to 'DistanceMonitorAP' network");
    Serial.println("Then visit http://" + myIP.toString() + " in your browser");
}

void loop() {
    webSocket.loop();
    server.handleClient();
    
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 100) { // Update every 100ms
        lastUpdate = millis();
        
        // Measure distance in centimeters
        float distance = getDistance();
        
        // Send to all connected WebSocket clients
        String json = "{\"distance\":" + String(distance) + "}";
        webSocket.broadcastTXT(json);
    }
}

float getDistance() {
    // Send pulse
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    // Measure echo duration in microseconds
    long duration = pulseIn(echoPin, HIGH);
    
    // Calculate distance in cm (speed of sound is 0.034 cm/μs)
    // Divide by 2 because sound travels to object and back
    float distance = duration * 0.034 / 2;
    
    // Filter out invalid readings (HC-SR04 range is 2cm-400cm)
    if (distance < 2 || distance > 400) {
        distance = 0; // Will display as 0.00 on webpage
    }
    
    return distance;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
                // Send current unit preference to new client
                webSocket.sendTXT(num, "cm");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] Received text: %s\n", num, payload);
            // Here you could handle unit change requests if needed
            break;
    }
}
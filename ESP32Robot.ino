#include <WiFi.h>
#include <WebServer.h>
#include "HardwareSerial.h"
#include "DataPacket.h"
#include "TrinamicStepper.h"

// Replace with your network credentials
const char* ssid = "Lac Duong";
const char* password = "linhtuoi";

// Web server on port 80
WebServer webServer(80);

// TCP server on port 1234
WiFiServer tcpServer(1234);
WiFiClient tcpClient;

#define MOTOR_NUM 3     // Numbe of the motor module
#define PORT_LED  2     // Led Port

int RunSpeed = 10; // default 50%
int SetSpeeds[MOTOR_NUM] = {0,0,0};
int SetDistances[MOTOR_NUM] = {0,0,0};
int MotorId = 1;
unsigned long previousMillis = 0;    // will store last time LED was updated
const long    MotorInterval = 24;       // interval at which to blink (milliseconds)
const long    interval = 24;           // interval at which to blink (milliseconds)


DataPacket dt;
//WiFiServer server(1234); // Create a TCP server on port 1234
TrinamicStepper stepper(Serial2);

// HTML page for control
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Omni Wheel Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; padding: 20px; }
    button { width: 120px; height: 40px; margin: 10px; font-size: 16px; }
    input[type=range] { width: 80%; }
  </style>
</head>
<body>
  <h2>Omni Wheel Control</h2>
  <p><strong>Run Speed: <span id="speedValue">10</span>%</strong></p>
  <input type="range" min="0" max="400" value="10" id="speedSlider" oninput="updateSpeed(this.value)" onchange="sendSpeed(this.value)">
  <br><br>
  <button onclick="sendCommand('rotateLeft')">Rotate Left</button>
  <button onclick="sendCommand('rotateRight')">Rotate Right</button><br>
  <button onclick="sendCommand('goUp')">Go Up</button>
  <button onclick="sendCommand('goDown')">Go Down</button><br>
  <button onclick="sendCommand('stop')">Stop</button>

  <script>
    function updateSpeed(val) {
      document.getElementById('speedValue').textContent = val;
    }
    function sendSpeed(val) {
      fetch("/setSpeed?value=" + val);
    }
    function sendCommand(cmd) {
      fetch("/command?do=" + cmd);
    }
  </script>
</body>
</html>
)rawliteral";

// ========== Web Server Routes ==========
void handleRoot() {
  webServer.send(200, "text/html", htmlPage);
}

void handleSetSpeed() {
  if (webServer.hasArg("value")) {
    RunSpeed = webServer.arg("value").toInt();
    Serial.printf("RunSpeed = %d\n", RunSpeed);
  }
  webServer.send(200, "text/plain", "OK");
}

void handleCommand() {
  if (!webServer.hasArg("do")) {
    webServer.send(400, "text/plain", "Missing command");
    return;
  }

  String cmd = webServer.arg("do");

  if (cmd == "rotateLeft") {
    SetSpeeds[0] = RunSpeed;
    SetSpeeds[1] = RunSpeed;
    SetSpeeds[2] = RunSpeed;
  } else if (cmd == "rotateRight") {
    SetSpeeds[0] = -RunSpeed;
    SetSpeeds[1] = -RunSpeed;
    SetSpeeds[2] = -RunSpeed;
  } else if (cmd == "goUp") {
    SetSpeeds[0] = 0;
    SetSpeeds[1] = -RunSpeed;
    SetSpeeds[2] = RunSpeed;
  } else if (cmd == "goDown") {
    SetSpeeds[0] = 0;
    SetSpeeds[1] = RunSpeed;
    SetSpeeds[2] = -RunSpeed;
  } else if (cmd == "stop") {
    SetSpeeds[0] = 0;
    SetSpeeds[1] = 0;
    SetSpeeds[2] = 0;
  }

  Serial.printf("Command: %s → Speeds = [%d, %d, %d]\n",
                cmd.c_str(), SetSpeeds[0], SetSpeeds[1], SetSpeeds[2]);

  webServer.send(200, "text/plain", "OK");
}


void setup() {
  // Initialize SoftwareSerial
  Serial.begin(38400);
  delay(100);  // Wait for stabilization

  for(MotorId = 1; MotorId<= MOTOR_NUM; MotorId ++)
  {
    stepper.DisableRightSwitchStop(MotorId);
    delay(MotorInterval);  // Wait for stabilization
    Serial.println(stepper.getStatus());
    stepper.DisableLeftSwitchStop(MotorId);
    delay(MotorInterval);  // Wait for stabilization
    stepper.setMaxPositionSpeed(MotorId, 200, true);
    delay(MotorInterval);  // Wait for stabilization
    stepper.setMaxAcceleration(MotorId, 10, true);
    delay(MotorInterval); 
    Serial.println(stepper.getStatus());
  }
  pinMode(PORT_LED,OUTPUT);

/*
  // Start WiFi AP
  WiFi.softAP(ssid, password);
  Serial.println("Access Point started.");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
*/
    // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Web server routes
  webServer.on("/", handleRoot);
  webServer.on("/setSpeed", handleSetSpeed);
  webServer.on("/command", handleCommand);
  webServer.begin();
  Serial.println("Web server started.");

  // Start TCP server
  tcpServer.begin();
  Serial.println("TCP server started on port 1234.");

}

int v = 100;
void loop() {

    webServer.handleClient();

  // Handle incoming TCP client
  if (!tcpClient || !tcpClient.connected()) {
    tcpClient = tcpServer.available();
  } else if (tcpClient.available()) {
    char buffer[128];
    int len = tcpClient.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
    buffer[len] = '\0';
    Serial.print("TCP Received: ");
    Serial.println(buffer);

    // Respond with current motor speeds
    String response = "RunSpeed = " + String(RunSpeed) +
                      ", SetSpeeds = [" + String(SetSpeeds[0]) + ", " +
                      String(SetSpeeds[1]) + ", " + String(SetSpeeds[2]) + "]\n";
    tcpClient.print(response);
  }
   
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= MotorInterval) {
    previousMillis = currentMillis;
        if(MotorId == 3) v++;
        if(MotorId > 3) MotorId = 1;
        stepper.rotateRight(MotorId, SetSpeeds[MotorId-1], true);
        MotorId++;
        Serial.print("sent. received:");
        Serial.println(stepper.getStatus()); 
        //digitalWrite(PORT_LED,1 - digitalRead(PORT_LED));
    }
 
}

  /*
  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  // Start the server
  server.begin();
  Serial.println("TCP server started.");
*/


  /*
  delay(interval);
  stepper.rotateRight(MODULE_ADDRESS_1, v, true);
  delay(interval);
  Serial.print("sent. received:");
  Serial.println(stepper.getStatus());
  stepper.rotateRight(MODULE_ADDRESS_2, v, true);
  delay(interval);
  Serial.print("sent. received:");
  Serial.println(stepper.getStatus());
  stepper.rotateRight(MODULE_ADDRESS_3, v, true);
  Serial.print("sent. received:");
  Serial.println(stepper.getStatus());
  if(v > 100) v = 0;
  */

/*
void loop() {
  WiFiClient client = server.available(); // Check if a client has connected
  if (client) {
    Serial.println("New client connected.");
    while (client.connected()) {
      if (client.available() > 0) {
          char c = client.read();
          if(dt.Push(c)) 
          {
          Serial.print("Received: Mode = ");
          Serial.print(dt.GetValue(0)); Serial.print(",");
          Serial.print(dt.GetValue(1)); Serial.print(",");
          Serial.print(dt.GetValue(2)); Serial.print(",");
          Serial.println(dt.GetValue(3));


          if(dt.GetValue(0) == 0) // Velocity
          {
          delay(20);
          if(stepper.rotateRight(MODULE_ADDRESS_1, dt.GetValue(1), true)) 
              {Serial.print(dt.GetValue(1));Serial.print("OK1 ")  ; }
          delay(10);
          if(stepper.rotateRight(MODULE_ADDRESS_2, dt.GetValue(2), true)) 
              { Serial.print(dt.GetValue(2));Serial.print("OK2 ")  ;}
          delay(10);
          if(stepper.rotateRight(MODULE_ADDRESS_3, dt.GetValue(3), true)) 
              { Serial.print(dt.GetValue(3));Serial.println("OK3")  ;}
          } 
          
          else

          {
            delay(20);
          if(stepper.moveRelative(MODULE_ADDRESS_1, dt.GetValue(1), true)) 
              {Serial.print(dt.GetValue(1));Serial.print("OK1 ")  ; }
          delay(10);
          if(stepper.moveRelative(MODULE_ADDRESS_2, dt.GetValue(2), true)) 
              { Serial.print(dt.GetValue(2));Serial.print("OK2 ")  ;}
          delay(10);
          if(stepper.moveRelative(MODULE_ADDRESS_3, dt.GetValue(3), true)) 
              { Serial.print(dt.GetValue(3));Serial.println("OK3")  ;}
            
          }
          

          
          client.println("OK");
          }
      }

    //unsigned long currentMillis = millis();
    //if (currentMillis - previousMillis >= interval) {
    //previousMillis = currentMillis;
    //}

    }
    Serial.println("Client disconnected.");
    client.stop();
    //stepper.stopMotor(MODULE_ADDRESS_1, true);
    //stepper.stopMotor(MODULE_ADDRESS_2, true);
    //stepper.stopMotor(MODULE_ADDRESS_3, true);
    
  }
}
*/

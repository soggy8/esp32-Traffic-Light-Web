// Load Wi-Fi library
#include <WiFi.h>

// Network credentials Here
const char* ssid     = "test";
const char* password = "test12345";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Variables to store the current LED states
String statePin23 = "off";
String statePin22 = "off";
String statePin21 = "off";
// Output variable to GPIO pins
const int ledPin23 = 23;
const int ledPin22 = 22;
const int ledPin21 = 21;

// Traffic light control variables
bool isTrafficLightActive = false;
unsigned long trafficLightStartTime = 0;
const unsigned long redDuration = 4000; // Duration for red light (in milliseconds)
const unsigned long yellowDuration = 2000; // Duration for yellow light (in milliseconds)
const unsigned long greenDuration = 4000;  // Duration for green light (in milliseconds)

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds
const long timeoutTime = 2000;

void setup() {
  Serial.begin(9600);
  
  pinMode(ledPin23, OUTPUT);      // set the LED pin mode
  digitalWrite(ledPin23, 0);      // turn LED off by default
  pinMode(ledPin22, OUTPUT);      // set the LED pin mode
  digitalWrite(ledPin22, 0);      // turn LED off by default
  pinMode(ledPin21, OUTPUT);      // set the LED pin mode
  digitalWrite(ledPin21, 0);      // turn LED off by default

  WiFi.softAP(ssid, password);
  
  // Print IP address and start web server
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.softAPIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client

    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /23/on") >= 0) {
              statePin23 = "on";
              digitalWrite(ledPin23, HIGH);               // turns the LED on
            } else if (header.indexOf("GET /23/off") >= 0) {
              statePin23 = "off";
              digitalWrite(ledPin23, LOW);                //turns the LED off
            }
            
            if (header.indexOf("GET /22/on") >= 0) {
              statePin22 = "on";
              digitalWrite(ledPin22, HIGH);               // turns the LED on
            } else if (header.indexOf("GET /22/off") >= 0) {
              statePin22 = "off";
              digitalWrite(ledPin22, LOW);                //turns the LED off
            }

            if (header.indexOf("GET /21/on") >= 0) {
              statePin21 = "on";
              digitalWrite(ledPin21, HIGH);               // turns the LED on
            } else if (header.indexOf("GET /21/off") >= 0) {
              statePin21 = "off";
              digitalWrite(ledPin21, LOW);                //turns the LED off
            }

            // Check for traffic light command
            if (header.indexOf("GET /traffic") >= 0) {
              isTrafficLightActive = true;  // Activate traffic light sequence
              trafficLightStartTime = millis(); // Record start time
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            client.println("<style>html { font-family: monospace; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: yellowgreen; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 32px; margin: 2px; cursor: pointer;}");
            client.println(".button {background-color: gray;}");
            client.println(".button {background-color: gray;} </style></head>");

            client.println("<body><h1>ESP32 Web Server</h1>");
            client.println("<p>Control LED State</p>");

            if (statePin23 == "off") {
              client.println("<p><a href=\"/23/on\"><button class=\"button\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/23/off\"><button class=\"button button2\">ON</button></a></p>");
            }
            if (statePin22 == "off") {
              client.println("<p><a href=\"/22/on\"><button class=\"button\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/22/off\"><button class=\"button button2\">ON</button></a></p>");
            }
            if (statePin21 == "off") {
              client.println("<p><a href=\"/21/on\"><button class=\"button\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/21/off\"><button class=\"button button2\">ON</button></a></p>");
            }
            // Add button for traffic light
            client.println("<p><a href=\"/traffic\"><button class=\"button\">Start Traffic Light</button></a></p>");
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  // Traffic light logic
  if (isTrafficLightActive) {
    unsigned long elapsedTime = millis() - trafficLightStartTime;

    if (elapsedTime < redDuration) {
      // Turn on red light
      digitalWrite(ledPin23, HIGH);
      digitalWrite(ledPin22, LOW);
      digitalWrite(ledPin21, LOW);
      statePin23 = "on";
      statePin22 = "off";
      statePin21 = "off";
    } else if (elapsedTime < redDuration + yellowDuration) {
      // Turn on yellow light
      digitalWrite(ledPin23, LOW);
      digitalWrite(ledPin22, HIGH);
      digitalWrite(ledPin21, LOW);
      statePin23 = "off";
      statePin22 = "on";
      statePin21 = "off";
    } else if (elapsedTime < redDuration + yellowDuration + yellowDuration) {
      // Turn on yellow light
      digitalWrite(ledPin23, LOW);
      digitalWrite(ledPin22, LOW);
      digitalWrite(ledPin21, HIGH);
      statePin23 = "off";
      statePin22 = "off";
      statePin21 = "on";
    } else {
      // Reset traffic light
      digitalWrite(ledPin23, LOW);
      digitalWrite(ledPin22, LOW);
      digitalWrite(ledPin21, LOW);
      statePin23 = "off";
      statePin22 = "off";
      statePin21 = "off";
      isTrafficLightActive = false; // Deactivate traffic light
    }
  }
}

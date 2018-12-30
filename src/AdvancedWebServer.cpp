/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "WiFiPrivateInfo.h"

// Define more friendly names for hardware pins based on GPIO numbers
#define LED_GREEN 14 // GPIO14, D5
#define LED_RED 12 // GPIO12, D6
#define LED_BLUE 13 // GPIO13, D7
#define LED_WHITE 15 // GPIO15, D8

//define onboard LED pins to avoid ZZ
#define LED_ONBOARD_BLUE 2
#define LED_ONBOARD_RED 16

const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WebServer server(80);

const int led = 13;

uint8_t redLevel, greenLevel, blueLevel, whiteLevel;

void drawGraph();

void handleRoot() {
  digitalWrite(led, 1);
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",

           hr, min % 60, sec % 60
          );
  server.send(200, "text/html", temp);
  digitalWrite(led, 0);
}

// void handleColorChosen() {
    
// }

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
    //initialize and turn off onboard LEDs
    pinMode(LED_ONBOARD_BLUE, OUTPUT);
    pinMode(LED_ONBOARD_RED, OUTPUT);
    digitalWrite(LED_ONBOARD_BLUE, HIGH);
    digitalWrite(LED_ONBOARD_RED, HIGH);

    //initialize relevant outputs
    analogWriteRange(255);
    pinMode(LED_RED, OUTPUT); 
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_WHITE, OUTPUT);
    
    // Start serial session and start WiFi connection
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("esp8266")) {
        Serial.println("MDNS responder started");
    }

    // https://www.esp8266.com/viewtopic.php?t=2153
    // Processing arguments of GET and POST requests is also easy enough. 
    // Let's make our sketch turn a led on or off depending on the value of a request argument.
    // http://<ip address>/led?state=on will turn the led ON
    // http://<ip address>/led?state=off will turn the led OFF
    //this is an inline callback
    server.on("/led", []() {
        String colorRGBW = server.arg("color");
        Serial.println("colorRGBW: " + colorRGBW);
        unsigned long colorRGBW_long = strtoul(colorRGBW.c_str(), NULL, 16);
        Serial.println("colorRGBW_long: " + colorRGBW_long);

        whiteLevel = colorRGBW_long & 0x000000FF;
        blueLevel = (colorRGBW_long >> 8) & 0x000000FF;
        greenLevel = (colorRGBW_long >> 16) & 0x000000FF;
        redLevel = (colorRGBW_long >> 24) & 0x000000FF;

        Serial.println(redLevel);
        Serial.println(greenLevel);
        Serial.println(blueLevel);
        Serial.println(whiteLevel);
        
        analogWrite(LED_RED, redLevel);
        analogWrite(LED_GREEN, greenLevel);
        analogWrite(LED_BLUE, blueLevel);
        analogWrite(LED_WHITE, whiteLevel);

        // if (colorRGBW == "on") digitalWrite(13, LOW);
        // else if (colorRGBW == "off") digitalWrite(13, HIGH);
        server.send(200, "text/plain", "Led is now on serial output");
    });
    server.on("/", handleRoot);
    server.on("/test.svg", drawGraph);
    server.on("/inline", []() {
        server.send(200, "text/plain", "this works as well");
    });
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}

void drawGraph() {
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send(200, "image/svg+xml", out);
}

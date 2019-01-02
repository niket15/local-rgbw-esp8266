/*
  A Simple Web Server for an ESP8266, to control an RGBW Analog LED Strip

  Output: Web Page on LAN
  Inputs: RGBW intensity in a HTTP GET request
  Output signals: PWM for each channel, corresponding to input

  TODO: POST requests with a more advanced UI
  TODO: HTTPS

  Note: ESP8266 output signals are 3.3V and are wired to corresponding IRLB8721 N-channel MOSFET gates (one per channel)


  Based off an Arduino ESP8266 Library Sketch called AdvancedWebServer:
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

uint8_t redLevel, greenLevel, blueLevel, whiteLevel;

ESP8266WebServer server(80);


void handleRoot() {
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='60'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"https://i.imgur.com/sBUyG2a.jpg\">\
  </body>\
</html>",

           hr, min % 60, sec % 60
          );
  server.send(200, "text/html", temp);
}

// void handleColorChosen() {
    
// }

void handleNotFound() {
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
}

void setup(void) {
    //initialize and turn off onboard LEDs which are active low
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
    
    // Start serial session (for debug) and start WiFi connection
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.hostname("LED_Control_0");
    WiFi.begin(ssid, password);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    //Connected...
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
        // Serial.println("colorRGBW_long: " + colorRGBW_long);

        whiteLevel = colorRGBW_long & 0x000000FF;
        Serial.println(whiteLevel);
        blueLevel = (colorRGBW_long >> 8) & 0x000000FF;
        Serial.println(blueLevel);
        greenLevel = (colorRGBW_long >> 16) & 0x000000FF;
        Serial.println(greenLevel);
        redLevel = (colorRGBW_long >> 24) & 0x000000FF;
        Serial.println(redLevel);

        
        analogWrite(LED_RED, redLevel);
        analogWrite(LED_GREEN, greenLevel);
        analogWrite(LED_BLUE, blueLevel);
        analogWrite(LED_WHITE, whiteLevel);

        // if (colorRGBW == "on") digitalWrite(13, LOW);
        // else if (colorRGBW == "off") digitalWrite(13, HIGH);
        server.send(200, "text/plain", "Led is now on serial output");
    });
    server.on("/", handleRoot);
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

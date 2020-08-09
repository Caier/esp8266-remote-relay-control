#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include "pins/pins_arduino.h"
#include "err.h"
#include "config.h"
#include "routes.h"

#define configFile "/config.json"

const uint8_t relay = D1,
              ledY = D2,
              ledG = D3,
              meter = A0;

unsigned long mil = 0;
bool reconnecting = false;
bool shouldRelayToggle = false;
int relayToggleLength = 0;

Config config;
AsyncWebServer server(80);

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    pinMode(relay, OUTPUT);
    pinMode(ledY, OUTPUT);
    pinMode(ledG, OUTPUT);
    pinMode(meter, INPUT);
    digitalWrite(ledY, 1);

    if(!SPIFFS.begin())
        err("An Error has occurred while mounting SPIFFS", 2, 1); // err.h
    if(!SPIFFS.exists(configFile))
        err("Can't find config file", 2, 2);
    else 
        loadConfig(configFile, &config); // config.h

    Serial.print("\nConnecting with: " + config.ssid);
    WiFi.config(config.staticIP, config.dns, config.gateway, config.subnet);
    WiFi.begin(config.ssid, config.password);
    Serial.println();
    while(1) {
        wl_status_t status = WiFi.status();
        if(status == WL_NO_SSID_AVAIL || status == WL_CONNECT_FAILED)
            err("Could not connect to the provided WiFi network", 1, 2);
        else if(status == WL_CONNECTED)
            break;
        Serial.print(".");
        delay(300);
    }

    Serial.println((String)"\nWiFi connected.\nIP address: " + WiFi.localIP().toString());

    setRoutes(&server); // routes.h
    server.begin();

    digitalWrite(ledY, 0);
    digitalWrite(ledG, 1);
}

void loop() {
    unsigned long nowmil = millis();

    if(nowmil - mil >= relayToggleLength && shouldRelayToggle) {
        digitalWrite(relay, !digitalRead(relay));
        digitalWrite(ledY, !digitalRead(ledY));
        mil = millis();
        if(!digitalRead(relay)) {
            shouldRelayToggle = false;
            mil = 0;
        }
    }

    if(WiFi.status() != WL_CONNECTED && !reconnecting) {
        reconnecting = true;
        digitalWrite(ledY, 1);
        digitalWrite(ledG, 0);
        Serial.println("Disconnected from WiFi...");
    } else if(WiFi.status() == WL_CONNECTED && reconnecting) {
        reconnecting = false;
        digitalWrite(ledY, 0);
        digitalWrite(ledG, 1);
        Serial.println("Reconnected to WiFi");
    }
}
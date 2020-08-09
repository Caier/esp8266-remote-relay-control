#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>

#define reqSUCC 200, "application/json", "{\"success\": true}"
#define respErr(msg) (String)"{\"success\": false, \"error\": \"" + msg + "\"}"
#define reqErr(msg) req->send(400, "application/json", respErr(msg))

extern int relayToggleLength;
extern bool shouldRelayToggle;
extern Config config;
extern const uint8_t meter;

void setRoutes(AsyncWebServer* server) {
    server->onNotFound([](AsyncWebServerRequest *req){
        req->send(404);
    });

    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html");
    });
    
    server->on("/state", HTTP_GET, [](AsyncWebServerRequest* req){
        req->send(200, "application/json", "{\"success\": true, \"state\": " + (String)analogRead(meter) + "}");
    });

    server->on("/relay", HTTP_POST, [](AsyncWebServerRequest *req){
        reqErr("empty request");
    },
    NULL,
    [](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t index, size_t total){
        if(shouldRelayToggle) {
            req->send(409, "application/json", respErr("Relay is currently toggled"));
            return;
        }
        String json = std::string((const char*)data).substr(0, len).c_str();
        StaticJsonDocument<500> jsdoc;
        DeserializationError deserr = deserializeJson(jsdoc, json);
        if(!deserr) {
            if(!jsdoc["length"]) {
                reqErr("Request object must contain a 'length' property");
                return;
            }
            if(!jsdoc["secret"]) {
                reqErr("Request object must contain a 'secret' property");
                return;
            }
            String secret = (const char*)jsdoc["secret"];
            if(secret == config.secret) {
                int length = (int)jsdoc["length"];
                if(length <= 20000) {
                    relayToggleLength = length;
                    shouldRelayToggle = true;
                    req->send(reqSUCC);
                }
                else
                    reqErr("Length too long.");
            } 
            else
                reqErr("Invalid secret.");
        }
        else
            reqErr(deserr.c_str());
    });
}
#include <ESP8266WiFi.h>
#include <Arduino.h>

struct Config
{
    String ssid;
    String password;
    IPAddress staticIP;
    IPAddress dns;
    IPAddress gateway;
    IPAddress subnet;
    String secret;
};

void loadConfig(String filename, Config *cfg) {
    File config = SPIFFS.open(filename, "r");
    if(!config)
        err("Can't open config file", 2, 3);
    StaticJsonDocument<1000> confDoc;
    DeserializationError deserr = deserializeJson(confDoc, config.readString());
    if(!deserr) {
        for(auto x : ((const char*[]){"ssid", "pass", "staticIP", "dns", "gateway", "subnet", "secret"}))
            if(!confDoc[x])
                err("Config file is missing one or more necessary fields", 2, 5);
        cfg->ssid = (const char*)confDoc["ssid"];
        cfg->password = (const char*)confDoc["pass"];
        cfg->staticIP.fromString((String)(const char*)confDoc["staticIP"]);
        cfg->dns.fromString((String)(const char*)confDoc["dns"]);
        cfg->gateway.fromString((String)(const char*)confDoc["gateway"]);
        cfg->subnet.fromString((String)(const char*)confDoc["subnet"]);
        cfg->secret = (const char*)confDoc["secret"];
    }
    else
        err("Can't parse config file", 2, 4);
}
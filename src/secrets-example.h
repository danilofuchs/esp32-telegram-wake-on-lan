#ifndef __SECRETS_H__
#define __SECRETS_H__

#include <Arduino.h>

#define SECRET_WIFI_SSID "wifi"
#define SECRET_WIFI_PASSWORD "password"

#define SECRET_TELEGRAM_BOT_TOKEN "12345679:XXXXXXXXXXXXXXXXXXXXXXXXXXXX"

struct targetDevice
{
    byte mac[6]; //The targets MAC address
    String deviceName;
};

// Add or remove devices from this list
// To get mac address On windows,
//         - open cmd
//         - type "ipconfig /all"
//         - copy the value for "physical address" (highlight and right click to copy)

targetDevice devices[] = {
    {{0xBC, 0x5F, 0xF4, 0xFF, 0xFF, 0xFF}, "Old PC"}, //BC-5F-F4-FF-FF-FF
    {{0x04, 0xD9, 0xF5, 0xFF, 0xFF, 0xFF}, "New PC"}  //04-D9-F5-FF-FF-FF
};

#endif
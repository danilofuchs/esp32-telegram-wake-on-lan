#ifndef __SECRETS_H__
#define __SECRETS_H__

#include <Arduino.h>
#include <WiFi.h>

#define SECRET_WIFI_SSID "wifi"
#define SECRET_WIFI_PASSWORD "password"

#define SECRET_TELEGRAM_BOT_TOKEN "12345679:XXXXXXXXXXXXXXXXXXXXXXXXXXXX"

struct targetDevice {
  String name;
  byte mac[6];
  /// @brief Address to ping. If IPAddress() (0.0.0.0), no ping will be sent
  /// You may need to allow ICMP Echo in the firewall
  IPAddress ip;
};

// Add or remove devices from this list
// To get mac address On windows,
//         - open cmd
//         - type "ipconfig /all"
//         - find the network adapter
//         - copy the value for "physical address" (highlight and right click to
//         copy)
// For ping to work, the device IP must be static

targetDevice devices[] = {
    {
        .name = "Old PC",
        .mac = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
        .ip = IPAddress(192, 168, 1, 1),
    },
    {
        .name = "New PC",
        .mac = {0xff, 0xff, 0xff, 0xff, 0xff, 0xfe},
        .ip = IPAddress(),
    },
};

#endif
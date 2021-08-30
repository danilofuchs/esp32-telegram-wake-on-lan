# ESP32 Telegram Wake On Lan

Performs a Wake On Lan (WOL) on a target device using an ESP32,
using a Telegram bot to remotely start the process.

The ESP32 must be connected to the same WiFi network as the target device and must
have internet connection to Telegram servers.

Uses PlatformIO in VSCode to build and upload.

This program was tested on the NodeMCU ESP32-S module, but should work on any ESP32.
Just make sure you choose the correct libraries on PlatformIO before compiling.

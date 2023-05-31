# ESP32 Telegram Wake On Lan

Turn your PC on from anywhere using Telegram and ESP32.

![Telegram chat with the WOL bot waking up a PC](./docs/demo.jpeg)

Performs a Wake On Lan (WOL) on a target device. The ESP32 must be connected to
the same Wi-Fi network as the target device and must
have internet connection to Telegram servers.

This program was tested on the NodeMCU ESP32-S module, but should work on any ESP32.
Just make sure you choose the correct libraries on PlatformIO before compiling.

## Setup

First, make sure your target device supports Wake On Lan.
You may need to enable it in the BIOS and/or your Network Card's Device Manager.
You may also need to disable Windows Fast Startup.

Check this guide for some troubleshooting: https://www.windowscentral.com/software-apps/windows-11/how-to-enable-wake-on-lan-on-windows-11

You should test it with any generic Wake On Lan app for Android or iOS before
attempting to automated it with an ESP32.

For pinging, the target computer must have an static IP address.

### Secrets file

Copy src/secrets.example.h to src/secrets.h and replace the values for:

- Wi-Fi network
- Telegram Bot
- Target device MAC address
- Target device IP address (Optional, for pings, requires static IP)

### Telegram Bot

Create a Telegram bot using [BotFather](https://t.me/botfather) and get the API token.

### Development environment

Install Platform.io for VSCode and install dependencies.

You may need to install drivers for the ESP32. Mine uses a CP210x USB-to-UART bridge.
You can find the drivers here: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers

Just build the project and upload it to the ESP32.

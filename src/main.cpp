#include <Arduino.h>

#include "secrets.h"

/*******************************************************************
    Send a WOL to a machine triggered from Telegram
    Wake on Lan is a standard for waking computers out of a power
    saving mode.
    Uses just an ESP8266
    By Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/

// ----------------------------
// Standard Libraries - Already Installed if you have ESP8266 set up
// ----------------------------

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUDP.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

// #include <WakeOnLan.h>
// Library for sending the WOL magic packet
// Needs to be installed from GitHub
// https://github.com/koenieee/WakeOnLan-ESP8266

#include <UniversalTelegramBot.h>
// #include <ArduinoJson.h>

char ssid[] =
    SECRET_WIFI_SSID;  // Set in secrets.h, use secrets-example.h as a template
char password[] = SECRET_WIFI_PASSWORD;  // Set in secrets.h

String telegram_bot_token = SECRET_TELEGRAM_BOT_TOKEN;  // Set in secrets.h

//------- ---------------------- ------

WiFiUDP UDP;
/**
 * This will brodcast the Magic packet on your entire network range.
 */
IPAddress computer_ip(255, 255, 255, 255);

void sendWOL();

// This is the Wifi client that supports HTTPS
WiFiClientSecure client;
UniversalTelegramBot bot(telegram_bot_token, client);

int delayBetweenChecks = 1000;
unsigned long lastTimeChecked;  // last time messages' scan has been done

static void configureWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.print("Connecting WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

static void configureLocalTime() {
  Serial.println("Retrieving updated UTC time: ");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600) {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println("");
  Serial.println("Time updated");
  Serial.print("Now: ");
  Serial.println(now);
}

static void configureTelegramBot() {
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  bot.longPoll = 5;

  const String commands =
      F("["
        "{\"command\":\"help\",  \"description\":\"Get bot usage help\"},"
        "{\"command\":\"start\", \"description\":\"Message sent when you open "
        "a chat with a bot\"},"
        "{\"command\":\"wol\",\"description\":\"Wake on Lan\"}"
        "]");
  bot.setMyCommands(commands);
}

enum MessageType {
  start,
  help,
  wol_requested,
  wol_target_selected,
  unknown,
};

static MessageType parseMessageType(telegramMessage message) {
  if (message.type == F("callback_query")) {
    return wol_target_selected;
  } else if (message.text == F("/wol")) {
    return wol_requested;
  } else if (message.text == F("/start")) {
    return start;
  }
  return unknown;
}

static void handleStart(telegramMessage message) {
  String chat_id = String(message.chat_id);
  bot.sendMessage(chat_id, "/wol : returns list of devices to send WOL to\n",
                  "Markdown");
}

static void handleMessage(telegramMessage message) {
  Serial.println(message.text);
  // switch (parseMessageType(message)) {
  //   case start:
  //     handleStart(message);
  //     break;
  //   case wol_requested:
  //     handleWolRequested(message);
  //     break;
  //   case wol_target_selected:
  //     handleWolTargetSelected(message);
  //     break;
  //   default:
  //     Serial.println("Unknown message type");
  //     break;
  // }
}

static void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    handleMessage(bot.messages[i]);
  }
}

void setup() {
  Serial.begin(115200);

  configureWiFi();
  configureLocalTime();
  configureTelegramBot();

  UDP.begin(9);
}

void loop() {
  if (millis() < lastTimeChecked + delayBetweenChecks) {
    return;
  }

  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  if (numNewMessages) {
    Serial.println("Has new messages");
    handleNewMessages(numNewMessages);
  }

  lastTimeChecked = millis();
}
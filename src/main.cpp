#include <Arduino.h>

#include "secrets.h"

/*******************************************************************
    Send a WOL to a machine triggered from Telegram
    Wake on Lan is a standard for waking computers out of a power
    saving mode.

    Based heavily on:
    https://github.com/witnessmenow/ESP8266-WOL-From-Telegram
 *******************************************************************/

// WiFi dependencies
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUDP.h>

// Telegram dependencies
#include <UniversalTelegramBot.h>

// WOL dependencies
#include <WakeOnLan.h>

// Ping dependencies
#include <ESP32Ping.h>

#include <list>

char ssid[] =
    SECRET_WIFI_SSID;  // Set in secrets.h, use secrets-example.h as a template
char password[] = SECRET_WIFI_PASSWORD;  // Set in secrets.h

String telegram_bot_token = SECRET_TELEGRAM_BOT_TOKEN;  // Set in secrets.h

//------- ---------------------- ------

WiFiUDP UDP;
WakeOnLan WOL(UDP);

// This is the Wifi client that supports HTTPS
WiFiClientSecure client;
UniversalTelegramBot bot(telegram_bot_token, client);

int delay_between_checks = 1000;
unsigned long last_time_checked;  // last time messages' scan has been done

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

static void configureWakeOnLan() {
  UDP.begin(9);

  WOL.setRepeat(3, 100);
  IPAddress broadcast =
      WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());

  Serial.println("");
  Serial.print("Broadcast address: ");
  Serial.println(broadcast.toString());
}

static void configureTelegramBot() {
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  bot.longPoll = 5;

  const String commands =
      F("["
        "{\"command\":\"help\",  \"description\":\"Get bot usage help\"},"
        "{\"command\":\"start\", \"description\":\"Message sent when you open "
        "a chat with a bot\"},"
        "{\"command\":\"wol\",\"description\":\"Wake on Lan\"},"
        "{\"command\":\"ping\",\"description\":\"Ping a host\"}"
        "]");
  if (!bot.setMyCommands(commands)) {
    Serial.println("Failed to set bot commands");
  }
}

enum MessageType {
  start,
  help,
  wol_requested,
  wol_target_selected,
  ping_requested,
  ping_target_selected,
  unknown,
};

static MessageType parseMessageType(telegramMessage message) {
  if (message.type == F("callback_query")) {
    if (message.text.startsWith("/wol")) {
      return MessageType::wol_target_selected;
    } else if (message.text.startsWith("/ping")) {
      return MessageType::ping_target_selected;
    }
  } else if (message.text == F("/start")) {
    return MessageType::start;
  } else if (message.text == F("/help")) {
    return MessageType::help;
  } else if (message.text == F("/wol")) {
    return MessageType::wol_requested;
  } else if (message.text == F("/ping")) {
    return MessageType::ping_requested;
  }
  return MessageType::unknown;
}

static void handleStart(telegramMessage message) {
  String chat_id = String(message.chat_id);
  bot.sendMessage(chat_id, "Hello there! Time to wake some PCs\n", "Markdown");
}

static void handleHelp(telegramMessage message) {
  String chat_id = String(message.chat_id);
  bot.sendMessage(chat_id,
                  "/start : Say hello!\n"
                  "/help  : This message\n"
                  "/wol   : Wake on LAN\n"
                  "/ping  : Ping a host\n",
                  "Markdown");
}

// Sends a prompt to select the target device using an inline keyboard input
static void handleWolRequested(telegramMessage message) {
  Serial.println("WOL requested");

  int number_of_devices = sizeof devices / sizeof *devices;

  // Keyboard Json is an array of arrays.
  // The size of the main array is how many row options the keyboard has
  // The size of the sub arrays is how many column that row has

  // The "text" property is what shows up in the keyboard
  // The "callback_data" property is the text that gets sent to us when pressed
  String json = "[";
  for (int i = 0; i < number_of_devices; i++) {
    // clang-format off
    json += "[{ \"text\" : \"" + devices[i].name + "\", \"callback_data\" : \"/wol " + String(i) + "\" }]";
    // clang-format on
    if (i + 1 < number_of_devices) {
      json += ",";
    }
  }
  json += "]";
  bot.sendMessageWithInlineKeyboard(message.chat_id, "Which device to wake?",
                                    "", json);
}

static void handleWolTargetSelected(telegramMessage message) {
  Serial.println("WOL target selected");

  String text = message.text;

  text.replace("/wol ", "");
  int index = text.toInt();

  int number_of_devices = sizeof devices / sizeof *devices;

  if (index < 0 || index >= number_of_devices) {
    Serial.println("Invalid device index");
    bot.answerCallbackQuery(message.query_id, "❌ Unknown device");
    return;
  }

  targetDevice device = devices[index];

  Serial.print("Sending WOL to: ");
  Serial.println(device.name);
  WOL.sendMagicPacket(device.mac, sizeof(device.mac));

  bot.answerCallbackQuery(message.query_id,
                          "✅ " + device.name + " is now awake (hopefully)\n");
}

static void handlePingRequested(telegramMessage message) {
  Serial.println("Ping requested");

  int number_of_devices = sizeof devices / sizeof *devices;

  // Keyboard Json is an array of arrays.
  // The size of the main array is how many row options the keyboard has
  // The size of the sub arrays is how many column that row has

  // The "text" property is what shows up in the keyboard
  // The "callback_data" property is the text that gets sent to us when pressed
  std::list<targetDevice> pingableDevices;

  for (int i = 0; i < number_of_devices; i++) {
    if (devices[i].ip == IPAddress()) {
      continue;
    }
    pingableDevices.push_back(devices[i]);
  }

  String json = "[";
  for (int i = 0; i < pingableDevices.size(); i++) {
    // clang-format off
    json += "[{ \"text\" : \"" + devices[i].name + "\", \"callback_data\" : \"/ping " + devices[i].ip.toString() + "\" }]";
    // clang-format on
    if (i + 1 < pingableDevices.size()) {
      json += ",";
    }
  }
  json += "]";
  bot.sendMessageWithInlineKeyboard(message.chat_id, "Which device to Ping?",
                                    "", json);
}

static void handlePingTargetSelected(telegramMessage message) {
  Serial.println("Ping target selected");

  String text = message.text;

  text.replace("/ping ", "");
  IPAddress ip = IPAddress();

  if (!ip.fromString(text)) {
    bot.answerCallbackQuery(message.query_id, "❌ Invalid IP address\n");
    Serial.println("Invalid IP address");
    return;
  }

  int number_of_devices = sizeof devices / sizeof *devices;

  for (int i = 0; i < number_of_devices; i++) {
    if (devices[i].ip == ip) {
      Serial.print("Pinging: ");
      Serial.println(devices[i].name);
      Serial.println(ip.toString());
      if (Ping.ping(ip)) {
        bot.answerCallbackQuery(message.query_id,
                                "✅ " + devices[i].name + " is alive\n");
      } else {
        bot.answerCallbackQuery(message.query_id,
                                "💀 " + devices[i].name + " is dead\n");
      }
      return;
    }
  }

  bot.answerCallbackQuery(message.query_id, "❌ Unknown device\n");
  Serial.println("Invalid device index");
}

static void handleMessage(telegramMessage message) {
  Serial.println(message.text);
  switch (parseMessageType(message)) {
    case MessageType::start:
      handleStart(message);
      break;
    case MessageType::help:
      handleHelp(message);
      break;
    case MessageType::wol_requested:
      handleWolRequested(message);
      break;
    case MessageType::wol_target_selected:
      handleWolTargetSelected(message);
      break;
    case MessageType::ping_requested:
      handlePingRequested(message);
      break;
    case MessageType::ping_target_selected:
      handlePingTargetSelected(message);
      break;
    default:
      Serial.println("Unknown message type");
      break;
  }
}

static void handleNewMessages(int num_new_messages) {
  for (int i = 0; i < num_new_messages; i++) {
    handleMessage(bot.messages[i]);
  }
}

void setup() {
  Serial.begin(115200);

  configureWiFi();
  configureLocalTime();
  configureWakeOnLan();
  configureTelegramBot();

  Serial.println("==== Listening for Telegram messages ====");
}

void loop() {
  if (millis() < last_time_checked + delay_between_checks) {
    return;
  }

  int num_new_messages = bot.getUpdates(bot.last_message_received + 1);

  if (num_new_messages) {
    Serial.println("Has new messages");
    handleNewMessages(num_new_messages);
  }

  last_time_checked = millis();
}
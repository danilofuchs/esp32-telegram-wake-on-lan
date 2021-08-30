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

// #include <UniversalTelegramBot.h>
// Library for interacting with the Telegram API
// Search for "Telegegram" in the Library manager and install
// The universal Telegram library
// https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot

// #include <ArduinoJson.h>
// Library used for parsing Json from the API responses
// NOTE: There is a breaking change in the 6.x.x version,
// install the 5.x.x version instead
// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson

// ----------------------------
// Replace the following!
// ----------------------------

char ssid[] = SECRET_WIFI_SSID;         // Set in secrets.h, use secrets-example.h as a template
char password[] = SECRET_WIFI_PASSWORD; // Set in secrets.h

char telegram_bot_token[] = SECRET_TELEGRAM_BOT_TOKEN; // Set in secrets.h

// Change to match how many devices are in the above array.
int numDevices = 2;

//------- ---------------------- ------

WiFiUDP UDP;
/**
 * This will brodcast the Magic packet on your entire network range.
 */
IPAddress computer_ip(255, 255, 255, 255);

void sendWOL();

// This is the Wifi client that supports HTTPS
WiFiClientSecure client;
// UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, client);

int delayBetweenChecks = 1000;
unsigned long lastTimeChecked; //last time messages' scan has been done

void setup()
{
  Serial.begin(115200);

  // Set WiFi to station mode and disconnect from an AP if it was Previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Required on 2.5 Beta or above.
  client.setInsecure();

  // longPoll keeps the request to Telegram open for the given amount of seconds if there are no messages
  // This hugely improves response time of the bot, but is only really suitable for projects
  // where the the initial interaction comes from Telegram as the requests will block the loop for
  // the length of the long poll
  // bot.longPoll = 60;

  UDP.begin(9);
}

// void handleNewMessages(int numNewMessages)
// {

//   for (int i = 0; i < numNewMessages; i++)
//   {

//     // If the type is a "callback_query", a inline keyboard button was pressed
//     if (bot.messages[i].type == F("callback_query"))
//     {
//       String text = bot.messages[i].text;
//       Serial.print("Call back button pressed with text: ");
//       Serial.println(text);

//       if (text.startsWith("WOL"))
//       {
//         text.replace("WOL", "");
//         int index = text.toInt();
//         Serial.print("Sending WOL to: ");
//         Serial.println(devices[index].deviceName);
//         WakeOnLan::sendWOL(computer_ip, UDP, devices[index].mac, sizeof devices[index].mac);
//       }
//     }
//     else
//     {
//       String chat_id = String(bot.messages[i].chat_id);
//       String text = bot.messages[i].text;

//       if (text == F("/wol"))
//       {

//         // Keyboard Json is an array of arrays.
//         // The size of the main array is how many row options the keyboard has
//         // The size of the sub arrays is how many columns that row has

//         // "The Text" property is what shows up in the keyboard
//         // The "callback_data" property is the text that gets sent when pressed
//         String keyboardJson = "[";
//         for (int i = 0; i < numDevices; i++)
//         {
//           keyboardJson += "[{ \"text\" : \"" + devices[i].deviceName + "\", \"callback_data\" : \"WOL" + String(i) + "\" }]";
//           if (i + 1 < numDevices)
//           {
//             keyboardJson += ",";
//           }
//         }
//         keyboardJson += "]";
//         bot.sendMessageWithInlineKeyboard(chat_id, "Send WOL to following devices:", "", keyboardJson);
//       }

//       // When a user first uses a bot they will send a "/start" command
//       // So this is a good place to let the users know what commands are available
//       if (text == F("/start"))
//       {

//         bot.sendMessage(chat_id, "/wol : returns list of devices to send WOL to\n", "Markdown");
//       }
//     }
//   }
// }

void loop()
{
  // if (millis() > lastTimeChecked + delayBetweenChecks)
  // {

  //   // getUpdates returns 1 if there is a new message from Telegram
  //   int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  //   if (numNewMessages)
  //   {
  //     Serial.println("got response");
  //     handleNewMessages(numNewMessages);
  //   }

  //   lastTimeChecked = millis();
  // }
}
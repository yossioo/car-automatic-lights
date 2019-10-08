#include <Arduino.h>
//#include <ESPAsyncTCP.h>
//#include <ESPAsyncWebServer.h>
//#include <ESPDash.h>

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "arduino_secrets.h"
#include <ESP8266WebServer.h>
#include "index.h" //Our HTML webpage contents with javascripts



// Initialize Wifi connection to the router
char ssid[] = SECRET_SSID;     // your network SSID (name)
char password[] = SECRET_PASS; // your network key

// Initialize Telegram BOT
#define BOTtoken SECRET_BOT_TOKEN  // your Bot Token (Get from Botfather)

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done
bool Start = false;

const int ledPin = LED_BUILTIN;
int ledStatus = 0;

ESP8266WebServer server(80); //Server on port 80



//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
  String s = MAIN_page; //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}

void handleADC() {
  int a = analogRead(A0);
  String adcValue = String(a);
  server.send(200, "text/plane", adcValue); //Send ADC value only to client ajax request
}



void setup() {
  // Start serial
  delay(100);
  Serial.begin(115200);

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  client.setInsecure();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // attempt to connect to Wifi network:
  Serial.print("\n\nConnecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(ledPin, OUTPUT); // initialize digital ledPin as an output.
  delay(10);
  digitalWrite(ledPin, HIGH); // initialize pin as off
  //  bot._debug = true;
  Serial.println("Executing getMe");
  bot.getMe();
  Serial.print("Name: [");
  Serial.print(bot.name);
  Serial.print("]\tuserName: [");
  Serial.print(bot.userName);
  Serial.println("]");


  String up_message = String(bot.name + " is up.\nConnected to WiFi: __" + ssid
                             + "__\nLocal IP: " + WiFi.localIP().toString());
  Serial.println(String("Sending message to owner: ") + OWNER);
  Serial.println(up_message);
  bot.sendMessage(OWNER, up_message, "Markdown");
  Serial.println("Message sent.");


  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  //  server.on("/setLED", handleLED);
  server.on("/readADC", handleADC);


  server.begin();                  //Start server
  Serial.println("HTTP server started");
  Serial.println("Boot process complete.");
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages called");
  //  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    if ( chat_id != OWNER) {
      bot.sendMessage(chat_id, String("You are not the owner of this bot. Go away. " + chat_id), "Markdown");
      return;
    }
    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/ledon") {
      digitalWrite(ledPin, LOW );   // turn the LED on (HIGH is the voltage level)
      ledStatus = 1;
      //      bot.sendMessage(chat_id, "Led is ON", "");
    }

    else if (text == "/ledoff") {
      ledStatus = 0;
      digitalWrite(ledPin, HIGH);   // turn the LED off (LOW is the voltage level)
      //      bot.sendMessage(chat_id, "Led is OFF", "");
    }

    else if (text == "/status") {
      if (ledStatus) {
        bot.sendMessage(chat_id, "Led is ON", "");
      } else {
        bot.sendMessage(chat_id, "Led is OFF", "");
      }
    }

    else if (text == "/start") {
      String welcome = "Welcome to Universal Arduino Telegram Bot library, " + from_name + ".\n";
      welcome += "This is Flash Led Bot example.\n\n";
      welcome += "/ledon : to switch the Led ON\n";
      welcome += "/ledoff : to switch the Led OFF\n";
      welcome += "/status : Returns current status of LED\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    } else {
      Serial.println("unknown message received:");
      Serial.println(text);
    }
  }
}

void loop() {
  if (millis() > Bot_lasttime + Bot_mtbs)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      //      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Bot_lasttime = millis();
  }
  server.handleClient();          //Handle client requests

}

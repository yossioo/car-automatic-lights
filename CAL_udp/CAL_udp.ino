#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include "arduino_secrets.h"
#include "helper_functions.h"
// Initialize Wifi connection to the router
char ssid[] = SECRET_SSID;     // your network SSID (name)
char password[] = SECRET_PASS; // your network key
WiFiClientSecure client;
const int ledPin = LED_BUILTIN;
const int BTN = 4;
int ledStatus = 0;

int red = 15;
int green = 12;
int blue = 13;
int LDRPin = A0;

WiFiUDP Udp;
IPAddress self, IP_Remote;

int localUdpPort = 55554;
int remoteUdpPort = 55555;


unsigned char bytes[4];

void setup() {
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
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  self = WiFi.localIP();
  Serial.println(self);

  IP_Remote = self;
  Serial.print("Target address: ");
  IP_Remote[3] = 255;
  Serial.println(IP_Remote);

  Udp.begin(localUdpPort);


  pinMode(BTN, INPUT);

}

void loop() {
  int btnVal = digitalRead(BTN);
  unsigned int brightnessVal = analogRead(LDRPin);
  int val = (btnVal) ? 127 : 145; //millis();
  Serial.printf("Sending %d in bytes:\n", val);
  Udp.beginPacket(IP_Remote, remoteUdpPort);
  uint32to4bytes(val, bytes);
  for (int bi = 0; bi < 4; ++bi) {
    Udp.write(bytes[bi]);
    Serial.print(bytes[bi], HEX);
  }
  
  uint32to4bytes(brightnessVal, bytes);
  for (int bi = 0; bi < 4; ++bi) {
    Udp.write(bytes[bi]);
    Serial.print(bytes[bi], HEX);
  }
  Udp.endPacket();
  Serial.println("\n");


if (brightnessVal < 500){
  analogWrite(red,0);
  analogWrite(green,0);
  analogWrite(blue,255);

} else {
  analogWrite(red,255);
  analogWrite(green,255);
  analogWrite(blue,0);
  
}
  
  delay(500);
}

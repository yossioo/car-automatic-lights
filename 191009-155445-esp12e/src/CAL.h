#ifndef CAL_HEADER
#define CAL_HEADER

#include "arduino_secrets.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP_EEPROM.h>
#include <StreamUtils.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>

#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1

typedef enum {
  LIGHTS_OFF_MOTOR = 0,
  LIGHTS_ON = 1,
  LIGHTS_DELAY = 2,
  LIGHTS_OFF_SENSOR = 3
} LightsMode;

struct State {
  float voltage; // V
  int sensor;    // Analog data - 0:1023 (1-1024?)
  bool motor_running;
  // char *ssid; // Wireless network ID
  String ssid_string;
  LightsMode lights_mode = LIGHTS_OFF_MOTOR;
  unsigned long int motor_start_millis;

  IPAddress ip_self;  /// The board itself
  IPAddress ip_bcast; /// Broadcast network;
};

struct Settings {
  int delay_lights_millis =
      3000; // Delay to turn lights on after the motor is running
  int sensor_threshold_to_turn_on = 300;
  int sensor_threshold_to_turn_off = 700;
  bool winter_mode = false;
  float acc_voltage_scaling_factor = 1;
  float motor_running_voltage_threshold = 13.5; // V
  int loop_delay = 100;
};

struct WiFi_Settings {
  int wifi_connect_timeout =
      15000; // Timeout connectiong to WiFi and create self AP
  bool use_bufferd_udp = false;
  char ssid[50] = SECRET_SSID;     // your network SSID (name)
  char password[50] = SECRET_PASS; // your network key
};

struct Parameters {
  int red = 15;
  int green = 12;
  int blue = 13;
  int sensorPin = A0;
  int localUdpPort = 55554;
  int remoteUdpPort = 55555;
};

class CAL {
public:
  CAL(){};
  ~CAL(){};
  void spin_once();
  void setup_things();
  void connect_to_wifi();
  State state;
  Settings settings;
  WiFi_Settings w_settings;
  Parameters params;

private:
  WiFiClientSecure client;
  WiFiUDP udp;

  void LoadSettings();
  void SaveSettings();
  void read_voltage();
  void read_sensor();
  void update_motor_state();
  void update_lights_state();
  void set_lights_mode(LightsMode lmode);
  void turnLights(bool on);
  void print_state_over_serial();
  void print_JSON_state_over_serial();
  void send_JSON_state_udp();
  void send_custom_message_udp(String msg);
  void receiveUdpPacket();
  void spin();
};

#endif
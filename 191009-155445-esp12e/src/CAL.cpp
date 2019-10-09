#include "CAL.h"
#include <Arduino.h>

void CAL::print_over_serial() { Serial.println("Hello from CAL"); }
void CAL::print_over_serial(const char *s) { Serial.println(s); }
void CAL::print_over_serial(int i) {
  Serial.print("CAL: ");
  Serial.println(i);
}

void CAL::print_state_over_serial() {
  Serial.print("CAL state:\n\tLIGHTS STATE: ");

  switch (this->state.lights_mode) {
  case LightsMode::LIGHTS_OFF_MOTOR:
    Serial.print("OFF_MOTOR");
    break;
  case LightsMode::LIGHTS_OFF_SENSOR:
    Serial.print("OFF_SENSOR");
    break;
  case LightsMode::LIGHTS_DELAY:
    Serial.print("DELAY");
    break;
  case LightsMode::LIGHTS_ON:
    Serial.print("ON");
    break;

  default:
    Serial.print("UNKNOWN");
    break;
  }

  Serial.print("\n\tVoltage: ");
  Serial.print(0.1 * this->state.voltage);
  Serial.print(" V\n\tMOTOR: ");
  Serial.print(this->state.motor_running ? "RUNNING" : "STOPPED");
  Serial.print("\n\tSensor: ");
  Serial.print(this->state.sensor);
  Serial.print("/1024  =  ");
  Serial.print(1.0 * this->state.sensor / 1024);
  Serial.print("%\n\tWifi: ");
  Serial.println(this->state.ssid_string);
  Serial.println("---------------");
}

void CAL::print_JSON_state_over_serial() {
  StaticJsonDocument<500> doc;
  doc["ssid"] = this->state.ssid_string;
  doc["voltage"] = this->state.voltage;
  JsonArray digitalValues = doc.createNestedArray("digital");
  digitalValues.add(0);
  digitalValues.add(1);
  serializeJson(doc, Serial);
  Serial.println("");

  Serial.println("---------------");
}

void CAL::read_voltage() {
  int pressed = !digitalRead(4);
  this->state.voltage = pressed ? 145 : 127;
}

void CAL::read_sensor() {
  this->state.sensor = analogRead(this->settings.sensorPin);
}

void CAL::update_motor_state() {
  if (this->state.voltage > this->settings.motor_running_voltage_threshold) {
    if (!this->state.motor_running) {
      this->state.motor_running = true;
      this->state.motor_start_millis = millis();
    }
  } else {
    this->state.motor_running = false;
  }
}

void debug_print_ligts(unsigned long int now, unsigned long int motor_start) {
  Serial.print("%\nmotor_start_millis: ");
  Serial.println(motor_start);
  Serial.print("%\n        now_millis: ");
  Serial.println(now);
  Serial.println("---------------");
}

void CAL::set_lights_mode(LightsMode lmode) { this->state.lights_mode = lmode; }

void CAL::update_lights_state() {
  /// HERE COMES THE MAIN LOGIC OF THE CAL
  if (!this->state.motor_running) {
    set_lights_mode(LightsMode::LIGHTS_OFF_MOTOR);
    return;
  }
  /// OK, the motor is running, let's think:
  switch (this->state.lights_mode) {
  case LightsMode::LIGHTS_OFF_MOTOR:
    if (this->state.motor_running) {
      set_lights_mode(LightsMode::LIGHTS_DELAY);
    }
    break;

  case LightsMode::LIGHTS_OFF_SENSOR:
    if (this->state.sensor < this->settings.sensor_threshold_to_turn_on) {
      set_lights_mode(LightsMode::LIGHTS_ON);
    }
    break;

  case LightsMode::LIGHTS_DELAY:
    if (this->state.sensor > this->settings.sensor_threshold_to_turn_off) {
      set_lights_mode(LightsMode::LIGHTS_OFF_SENSOR);
    } else {
      unsigned long int now = millis();
      if (now > this->state.motor_start_millis) {
        set_lights_mode(LightsMode::LIGHTS_ON);
      }
    }
    break;

  case LightsMode::LIGHTS_ON:
    if (this->state.sensor > this->settings.sensor_threshold_to_turn_off) {
      this->state.lights_mode = LightsMode::LIGHTS_OFF_SENSOR;
    }
    break;

  default:
    break;
  }
}

void CAL::turnLights(bool on) {
  if (on) {
    analogWrite(this->settings.red, 255);
    analogWrite(this->settings.green, 255);
    analogWrite(this->settings.blue, 255);
  } else {
    analogWrite(this->settings.red, 100);
    analogWrite(this->settings.green, 75);
    analogWrite(this->settings.blue, 0);
  }
}

void CAL::setup_things() {
  pinMode(4, INPUT);
  pinMode(this->settings.red, OUTPUT);
  pinMode(this->settings.green, OUTPUT);
  pinMode(this->settings.blue, OUTPUT);
}

void CAL::connect_to_wifi() {
  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  client.setInsecure();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // attempt to connect to Wifi network:
  Serial.print("\n\nConnecting Wifi: ");
  Serial.println(this->settings.ssid);
  WiFi.begin(this->settings.ssid, this->settings.password);

  unsigned long int wifi_start_millis = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    if (millis() > wifi_start_millis + this->settings.wifi_connect_timeout) {
      Serial.print("\nFAILED TO CONNECTO TO ");
      Serial.println(this->settings.ssid);
      Serial.println("Creating AP");
      WiFi.mode(WIFI_AP);
      WiFi.disconnect();
      delay(100);
      this->state.ssid_string = "ESP-CAL-AP";
      WiFi.softAP(this->state.ssid_string, "aaabbbcd");

      this->state.ip_self = WiFi.softAPIP();
      this->state.ip_bcast = this->state.ip_self;
      this->state.ip_bcast[3] = 255;
      Serial.print("AP IP address: ");
      Serial.println(this->state.ip_self);
      Serial.print("Broadcast address: ");
      Serial.println(this->state.ip_bcast);
      udp.begin(this->settings.localUdpPort);

      return;
    }
  }
  this->state.ssid_string = this->settings.ssid;
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  this->state.ip_self = WiFi.localIP();
  Serial.println(this->state.ip_self);

  this->state.ip_bcast = this->state.ip_self;
  Serial.print("Broadcast address: ");
  this->state.ip_bcast[3] = 255;
  Serial.println(this->state.ip_bcast);
  udp.begin(this->settings.localUdpPort);
}

void CAL::send_JSON_state_udp() {
  StaticJsonDocument<500> status;
  status["ssid"] = this->state.ssid_string;
  status["voltage"] = this->state.voltage;
  status["sensor"] = this->state.sensor;
  JsonArray thr_on_off = status.createNestedArray("thr_on_off");
  thr_on_off.add(this->settings.sensor_threshold_to_turn_on);
  thr_on_off.add(this->settings.sensor_threshold_to_turn_off);
  status["motor_running"] = this->state.motor_running;

  switch (this->state.lights_mode) {
  case LightsMode::LIGHTS_OFF_MOTOR:
    status["lights_mode"] = "OFF_MOTOR";
    break;
  case LightsMode::LIGHTS_OFF_SENSOR:
    status["lights_mode"] = "OFF_SENSOR";
    break;
  case LightsMode::LIGHTS_DELAY:
    status["lights_mode"] = "DELAY";
    break;
  case LightsMode::LIGHTS_ON:
    status["lights_mode"] = "ON";
    break;
  default:
    status["lights_mode"] = "UNKNOWN";
    break;
  }
  status["motor_start_millis"] = this->state.motor_start_millis;
  status["now_millis"] = millis();
  status["ip_self"] = this->state.ip_self.toString();

  status["motor_running_voltage_threshold"] =
      0.1 * this->settings.motor_running_voltage_threshold;
  status["delay_lights_millis"] = this->settings.delay_lights_millis;
  status["sensor_threshold_to_turn_on"] =
      this->settings.sensor_threshold_to_turn_on;
  status["sensor_threshold_to_turn_off"] =
      this->settings.sensor_threshold_to_turn_off;
  status["sensor_threshold_to_turn_off"] =
      this->settings.sensor_threshold_to_turn_off;
  status["wifi_connect_timeout"] = this->settings.wifi_connect_timeout;
  status["localUdpPort"] = this->settings.localUdpPort;
  status["remoteUdpPort"] = this->settings.remoteUdpPort;
  status["winter_mode"] = this->settings.winter_mode;
  udp.beginPacket(this->state.ip_bcast, this->settings.remoteUdpPort);

  if (this->settings.use_bufferd_udp) {
    WriteBufferingStream bufferedWifiClient(udp, 64);
    serializeJson(status, bufferedWifiClient);
    bufferedWifiClient.flush();
  } else {
    serializeJson(status, udp);
    udp.println();
  }
  udp.endPacket();
}
/// The method is called in loop() function
void CAL::spin_once() {
  read_voltage();
  read_sensor();
  update_motor_state();
  update_lights_state();
  // print_state_over_serial();
  // print_JSON_state_over_serial();

  send_JSON_state_udp();
}

void CAL::spin() {}
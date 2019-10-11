#include "CAL.h"
#include <Arduino.h>

void print_settings(Settings data) {
  Serial.printf("motor_running_voltage_threshold: %3.3f\n",
                data.motor_running_voltage_threshold);
  Serial.printf("delay_lights_millis: %d\n", data.delay_lights_millis);
  Serial.printf("sensor_threshold_to_turn_on: %d\n",
                data.sensor_threshold_to_turn_on);
  Serial.printf("sensor_threshold_to_turn_off: %d\n",
                data.sensor_threshold_to_turn_off);
  Serial.printf("acc_voltage_scaling_factor: %3.3f\n",
                data.acc_voltage_scaling_factor);
  Serial.printf("winter_mode: %d\n", data.winter_mode);
}

void print_WiFi_Settings(WiFi_Settings data) {
  Serial.printf("wifi_connect_timeout: %d\n", data.wifi_connect_timeout);
  Serial.printf("use_bufferd_udp: %d\n", data.use_bufferd_udp);
  Serial.printf("ssid: %s\n", data.ssid);
  Serial.printf("password: %s\n", data.password);
}

void CAL::LoadSettings() {
  int settings_size = sizeof(Settings);

  EEPROM.begin(settings_size);
  Settings data;
  EEPROM.get(0, data);

  Serial.print("EEPROM Settings: ");
  Serial.print(settings_size);
  Serial.println(" bytes");
  print_settings(data);
}

void CAL::SaveSettings() {
  EEPROM.put(0, settings);
  EEPROM.commit();
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
  this->state.voltage = pressed ? 14.5 : 12.7;
  /* Add something like
  int val = analogRead(A1);
  this->state.voltage = val * settings.acc_voltage_scaling_factor;
  */
}

void CAL::read_sensor() {
  this->state.sensor = analogRead(this->params.sensorPin);
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
    analogWrite(this->params.red, 255);
    analogWrite(this->params.green, 255);
    analogWrite(this->params.blue, 255);
  } else {
    analogWrite(this->params.red, 100);
    analogWrite(this->params.green, 75);
    analogWrite(this->params.blue, 0);
  }
}

void CAL::setup_things() {
  pinMode(4, INPUT);
  pinMode(this->params.red, OUTPUT);
  pinMode(this->params.green, OUTPUT);
  pinMode(this->params.blue, OUTPUT);

  LoadSettings();
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
  Serial.println(this->w_settings.ssid);
  WiFi.begin(this->w_settings.ssid, this->w_settings.password);

  unsigned long int wifi_start_millis = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    if (millis() > wifi_start_millis + this->w_settings.wifi_connect_timeout) {
      Serial.print("\nFAILED TO CONNECTO TO ");
      Serial.println(this->w_settings.ssid);
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
      Serial.print("Subnet mask: ");
      Serial.println(WiFi.subnetMask());
      Serial.print("Broadcast address: ");
      Serial.println(this->state.ip_bcast);
      udp.begin(this->params.localUdpPort);

      return;
    }
  }
  this->state.ssid_string = this->w_settings.ssid;
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  this->state.ip_self = WiFi.localIP();
  Serial.println(this->state.ip_self);
  Serial.print("Subnet mask: ");
  IPAddress sbnet = WiFi.subnetMask();
  Serial.println(sbnet);

  this->state.ip_bcast = this->state.ip_self;
  Serial.print("Broadcast address: ");
  this->state.ip_bcast[0] = state.ip_self[0] | (0xFF - sbnet[0]);
  this->state.ip_bcast[1] = state.ip_self[1] | (0xFF - sbnet[1]);
  this->state.ip_bcast[2] = state.ip_self[2] | (0xFF - sbnet[2]);
  this->state.ip_bcast[3] = state.ip_self[3] | (0xFF - sbnet[3]);
  // this->state.ip_bcast[3] = 255;
  Serial.println(this->state.ip_bcast);
  udp.begin(this->params.localUdpPort);
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
      this->settings.motor_running_voltage_threshold;
  status["delay_lights_millis"] = this->settings.delay_lights_millis;
  status["sensor_threshold_to_turn_on"] =
      this->settings.sensor_threshold_to_turn_on;
  status["sensor_threshold_to_turn_off"] =
      this->settings.sensor_threshold_to_turn_off;
  status["sensor_threshold_to_turn_off"] =
      this->settings.sensor_threshold_to_turn_off;
  status["wifi_connect_timeout"] = this->w_settings.wifi_connect_timeout;
  status["localUdpPort"] = this->params.localUdpPort;
  status["remoteUdpPort"] = this->params.remoteUdpPort;
  status["winter_mode"] = this->settings.winter_mode;
  udp.beginPacket(this->state.ip_bcast, this->params.remoteUdpPort);

  if (this->w_settings.use_bufferd_udp) {
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
  receiveUdpPacket();
}

void CAL::receiveUdpPacket() {
  /// Receive UDP:
  char incomingPacket[1024];
  int packetSize = udp.parsePacket();

  if (packetSize) {
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize,
                  udp.remoteIP().toString().c_str(), udp.remotePort());
    int len = udp.read(incomingPacket, packetSize);
    if (len > 0) {
      incomingPacket[len] = '\0';
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);

    // StaticJsonDocument<1000> new_settings;
    DynamicJsonDocument in_json(packetSize);

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(in_json, incomingPacket);

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    if (in_json.containsKey("ssid")) {
      Serial.println("Received new WiFi settings.");
    } else if (in_json.containsKey("winter_mode")) {
      Serial.println("Received new settings.");
    } else {
      Serial.println("Received UNKNOWN message type.");
    }
    // serializeJsonPretty(new_settings, Serial);
  }
}

void CAL::send_custom_message_udp(String msg) {}

void CAL::spin() {}
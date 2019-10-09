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
  Serial.print(this->state.lights_mode);
  Serial.print("\n\tVoltage: ");
  Serial.print(0.1 * this->state.voltage);
  Serial.print(" V\n\tMOTOR: ");
  Serial.print(this->state.motor_running ? "RUNNING" : "STOPPED");
  Serial.print("\n\tSensor: ");
  Serial.print(this->state.sensor);
  Serial.print("/1024  =  ");
  Serial.print(1.0 * this->state.sensor / 1024);
  Serial.print("%\n\tWifi: ");
  Serial.println(this->state.ssid);
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

void CAL::update_lights_state() {
  if (!this->state.motor_running) {
    this->state.lights_mode = Mode::LIGHTS_OFF;
  } else {
    unsigned long int now = millis();
    debug_print_ligts(now, this->state.motor_start_millis);
    if (now > this->state.motor_start_millis) {
      this->state.lights_mode = Mode::LIGHTS_ON;
    } else {
      this->state.lights_mode = Mode::LIGHTS_DELAY;
    }
  }
  setLights(this->state.lights_mode == LIGHTS_ON);
}

void CAL::setLights(bool on) {
  if (on) {
    analogWrite(this->settings.red, 0);
    analogWrite(this->settings.green, 0);
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

/// The method is called in loop() function
void CAL::spin_once() {
  read_voltage();
  read_sensor();
  update_motor_state();
  update_lights_state();
}

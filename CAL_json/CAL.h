#include <Arduino.h>

typedef enum { LIGHTS_OFF = 0, LIGHTS_ON = 1, LIGHTS_DELAY = 2 } Mode;

struct State {
  int voltage; // VOLT * 10
  int sensor;  // Analog data - 0:1023 (1-1024?)
  bool motor_running;
  char ssid[]; // Wireless network ID
  Mode lights_mode;
  unsigned long int motor_start_millis;
};

struct Settings {
  int motor_running_voltage_threshold = 135; // VOLT * 10
  int delay_lights_millis =
      3000; // Delay to turn lights on after the motor is running
  int sensor_threshold =
      100; // Sensor analog 10-bit value (0-1023) below which the lights should
           // be lit, given the motor is running.
  int sensorPin = A0;
  int red = 15;
  int green = 12;
  int blue = 13;
};

class CAL {
public:
  CAL(){};
  ~CAL(){};
  void print_over_serial();
  void print_over_serial(const char *s);
  void print_over_serial(int i);
  void print_state_over_serial();
  void read_voltage();
  void read_sensor();
  void spin_once();
  void update_motor_state();
  void update_lights_state();
  void setLights(bool on);
  void setup_things();

private:
  State state;
  Settings settings;
};

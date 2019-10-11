#include "CAL.h"
// #include "Ticker.h"

CAL *cal;
// Ticker spinner;

void cb() { Serial.println("spinner"); }
void setup() {
  // put your setup code here, to run once:
  delay(1);
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  cal = new CAL();
  cal->setup_things();
  analogWrite(cal->params.red, 128);
  analogWrite(cal->params.green, 75);
  analogWrite(cal->params.blue, 0);
  cal->connect_to_wifi();
  analogWrite(cal->params.red, 0);
  analogWrite(cal->params.green, 255);
  analogWrite(cal->params.blue, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(cal->settings.loop_delay);
  cal->spin_once();
}

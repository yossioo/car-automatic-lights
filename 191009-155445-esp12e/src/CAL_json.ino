#include "CAL.h"
#include "Ticker.h"

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
  analogWrite(cal->settings.red, 128);
  analogWrite(cal->settings.green, 75);
  analogWrite(cal->settings.blue, 0);
  cal->connect_to_wifi();
  analogWrite(cal->settings.red, 0);
  analogWrite(cal->settings.green, 255);
  analogWrite(cal->settings.blue, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
  cal->spin_once();
}

#include "CAL.h"

CAL* cal;

void setup() {
  // put your setup code here, to run once:
  delay(1);
  Serial.begin(115200);
  delay(1000);
  cal = new CAL();
  cal->setup_things();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(499);
  cal->spin_once();
  cal->print_state_over_serial();
}

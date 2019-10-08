
int red = 15;
int green = 12;
int blue = 13;
int LDRPin = A0;
boolean onOff = 1;
char* color;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(red, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(green, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  int r = -1;
  r = analogRead(LDRPin);
  Serial.print("Reading: ");
  Serial.println(r);

  if (r > 0) {
    color = "blue";
  } else {
    return;
  }
  if (r > 300) {
    color = "green";
  }
  if (r > 600) {
    color = "red";
  }
  ledCall(color, 255);
}


void ledCall(String colour, int strength) {
  analogWrite(red, 0);
  analogWrite(green, 0);
  analogWrite(blue, 0);
  if (onOff == true) {
    if (colour == "red") {
      analogWrite(red, strength);
    }

    if (colour == "green") {
      analogWrite(green, strength);
    }

    if (colour == "blue") {
      analogWrite(blue, strength);
    }
  }
}

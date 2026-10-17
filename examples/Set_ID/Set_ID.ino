#include <GTD35.h>

GTD35 driver(Serial1);

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
  driver.begin(115200, -1);
}

void loop() {
  driver.setID(3);
}

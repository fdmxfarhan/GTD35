#include <GTD35.h>
GTD35 driver(Serial1);

void setup() {
  driver.begin(115200);
  delay(200);
}

void loop() {
  driver.motor(100, 100, 100, 100);
  delay(1000);

  driver.motor(-100, -100, -100, -100);
  delay(1000);
}
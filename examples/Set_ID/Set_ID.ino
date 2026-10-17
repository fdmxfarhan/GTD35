#include <GTD35.h>

GTD35 driver(Serial1);

void setup() {
  driver.begin(115200, -1);
}

void loop() {
  driver.setID(3);
}

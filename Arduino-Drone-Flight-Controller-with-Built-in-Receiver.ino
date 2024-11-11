#include "nrf24l01.h"
#include "mpu6050.h"

void setup() {
  Serial.begin(9600);
  rxsetup();
  imusetup();
}


void loop() {
  rxloop();
  imuloop();
}

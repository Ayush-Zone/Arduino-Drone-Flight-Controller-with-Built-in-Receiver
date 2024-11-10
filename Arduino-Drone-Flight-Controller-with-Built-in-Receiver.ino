// 8 Channel PWM Receiver
// Author: Ayush Sharma
// GitHub: https://github.com/Ayush-Zone

#include <Wire.h>       // For I2C communication
#include <SPI.h>        // For SPI communication
#include <nRF24L01.h>   // Library for nRF24L01 radio module
#include <RF24.h>       // RF24 library for nRF24L01 communication
#include <Servo.h>      // Servo library for controlling servos

// PWM channel width variables
int ch_width_1 = 0;
int ch_width_2 = 0;
int ch_width_3 = 0;
int ch_width_4 = 0;
int ch_width_5 = 0;
int ch_width_6 = 0;
int ch_width_7 = 0;
int ch_width_8 = 0;

// Structure to store signal data for 8 channels
struct Signal {
  byte throttle;
  byte yaw;
  byte pitch;
  byte roll;
  byte aux1;
  byte aux2;
  byte aux3;
  byte aux4;
};

// Data object to hold received signal values
Signal data;

// nRF24L01 radio communication setup
const uint64_t pipeIn = 0xE9E8F0F0E1LL;  // Communication pipe address
RF24 radio(7, 10);                       // Radio object with CE and CSN pins

// Reset data to default values
void ResetData() {
  data.roll = 127;
  data.pitch = 127;
  data.throttle = 0;
  data.yaw = 127;
  data.aux1 = 0;
  data.aux2 = 0;
  data.aux3 = 0;
  data.aux4 = 0;
}

int LedPin = 8;   // LED pin to indicate communication status

// Gyroscope rate variables
float RateRoll, RatePitch, RateYaw;
float RateCalibrationRoll, RateCalibrationPitch, RateCalibrationYaw;
int RateCalibrationNumber;   // Counter for gyroscope calibration

// Function to read gyroscope signals
void gyro_signals(void) {
  // Configuring gyroscope settings via I2C
  Wire.beginTransmission(0x68);  // Gyroscope address
  Wire.write(0x1A);              // Set low-pass filter register
  Wire.write(0x05);              // Set low-pass filter to 5
  Wire.endTransmission();
  
  Wire.beginTransmission(0x68);  // Gyroscope address
  Wire.write(0x1B);              // Set gyro config register
  Wire.write(0x08);              // Set sensitivity to ±500°/s
  Wire.endTransmission();
  
  Wire.beginTransmission(0x68);  // Start gyro data read
  Wire.write(0x43);              // Begin reading gyro data
  Wire.endTransmission();
  Wire.requestFrom(0x68, 6);     // Request 6 bytes from gyroscope
  
  // Read 16-bit values for X, Y, and Z gyroscope data
  int16_t GyroX = Wire.read() << 8 | Wire.read();
  int16_t GyroY = Wire.read() << 8 | Wire.read();
  int16_t GyroZ = Wire.read() << 8 | Wire.read();
  
  // Convert raw gyro data to degrees per second
  RateRoll = (float)GyroX / 65.5;
  RatePitch = (float)GyroY / 65.5;
  RateYaw = (float)GyroZ / 65.5;
}

void setup() {
  Serial.begin(9600);             // Initialize serial communication for debugging
  radio.begin();                  // Start radio
  radio.startListening();         // Start listening for signals
  radio.openReadingPipe(1, pipeIn); // Open reading pipe
  radio.setAutoAck(false);        // Disable auto acknowledgment
  radio.setCRCLength(RF24_CRC_8); // Set CRC length to 8 bits
  radio.setDataRate(RF24_250KBPS);// Set data rate
  radio.setPALevel(RF24_PA_MAX);  // Set power amplifier level to max
  radio.setChannel(108);          // Set communication channel
  ResetData();                    // Initialize data to defaults
  
  pinMode(LedPin, OUTPUT);        // Set LED pin as output
  
  // Set up gyroscope for I2C communication
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  Wire.setClock(400000);          // Set I2C clock speed to 400 kHz
  Wire.begin();
  
  delay(250);                     // Small delay to allow setup stabilization
  Wire.beginTransmission(0x68);   // Wake up gyroscope
  Wire.write(0x6B);               // Power management register
  Wire.write(0x00);               // Exit sleep mode
  Wire.endTransmission();
  
  // Calibrate gyroscope over 2000 samples
  for (RateCalibrationNumber = 0; RateCalibrationNumber < 2000; RateCalibrationNumber++) {
    gyro_signals();
    RateCalibrationRoll += RateRoll;
    RateCalibrationPitch += RatePitch;
    RateCalibrationYaw += RateYaw;
    delay(1);                     // Delay to gather enough calibration samples
  }
  
  // Average calibration values
  RateCalibrationRoll /= 2000;
  RateCalibrationPitch /= 2000;
  RateCalibrationYaw /= 2000;
}

unsigned long lastRecvTime = 0;   // Last time data was received

// Function to receive data from the radio
void recvData() {
  while (radio.available()) {     // Check if radio has data available
    radio.read(&data, sizeof(Signal)); // Read data into the Signal struct
    lastRecvTime = millis();      // Update last received time
    digitalWrite(LedPin, HIGH);   // Turn on LED to indicate data reception
  }
}

void loop() {
  recvData();                     // Receive data from radio
  
  unsigned long now = millis();   // Get current time
  if (now - lastRecvTime > 1000) { // If no data for over 1 second
    ResetData();                  // Reset to default values
    digitalWrite(LedPin, LOW);    // Turn off LED to indicate data loss
  }

  // Map received data to PWM values (1000-2000 range for servos)
  ch_width_1 = map(data.roll, 0, 255, 1000, 2000);
  ch_width_2 = map(data.pitch, 0, 255, 1000, 2000);
  ch_width_3 = map(data.throttle, 0, 255, 1000, 2000);
  ch_width_4 = map(data.yaw, 0, 255, 1000, 2000);

  ch_width_5 = map(data.aux1, 0, 1, 1000, 2000);
  ch_width_6 = map(data.aux2, 0, 1, 1000, 2000);
  ch_width_7 = map(data.aux3, 0, 1, 1000, 2000);
  ch_width_8 = map(data.aux4, 0, 1, 1000, 2000);

  // Read and calibrate gyroscope signals
  gyro_signals();
  RateRoll -= RateCalibrationRoll;
  RatePitch -= RateCalibrationPitch;
  RateYaw -= RateCalibrationYaw;

  // Output channel widths and gyro data to serial monitor
  Serial.print(ch_width_1); Serial.print(" ");
  Serial.print(ch_width_2); Serial.print(" ");
  Serial.print(ch_width_3); Serial.print(" ");
  Serial.print(ch_width_4); Serial.print(" ");
  Serial.print(ch_width_5); Serial.print(" ");
  Serial.print(ch_width_6); Serial.print(" ");
  Serial.print(ch_width_7); Serial.print(" ");
  Serial.print(ch_width_8); Serial.print(" ");

  Serial.print("Roll rate [°/s]= "); Serial.print(RateRoll);
  Serial.print(" Pitch Rate [°/s]= "); Serial.print(RatePitch);
  Serial.print(" Yaw Rate [°/s]= "); Serial.println(RateYaw);

  delay(100);                     // Delay before next loop iteration
}

#ifndef MPU6050_H
#define MPU6050_H


// Global variables
extern float RateRoll, RatePitch, RateYaw;
extern float RateCalibrationRoll, RateCalibrationPitch, RateCalibrationYaw;
extern int RateCalibrationNumber;


// Function Prototypes
void gyro_signals();
void imusetup();
void imuloop();

#endif // RECEIVER_H

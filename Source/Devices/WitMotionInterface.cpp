//
//  m1-orientationmanager
//  Copyright © 2022 Mach1. All rights reserved.
//

#include "WitMotionInterface.h"

//==============================================================================
WitMotionInterface::WitMotionInterface()
{
}

WitMotionInterface::~WitMotionInterface()
{
}

float* WitMotionInterface::updateOrientation(char readBuffer[], int length){
    if(!isConnected){
        return nullptr;
    }

    for (int i = 0; i < length; i++) {
        parseData(readBuffer[i]);
    }

    float* newOrientation = getAngle();
    return newOrientation;
}

void WitMotionInterface::resetOrientation(){
    for(int i=0; i< 3; i++){
        magnetic_field_shift[i] = 0;
        acceleration_shift[i] = 0;
        gyro_shift[i] = 0;
        angle_shift[i] = 0;
    }
}

void WitMotionInterface::recenter(){
    // angle tare
    float* swpAngle = getAngle();
    angle_shift[1] = angle_shift[1] - swpAngle[0];
    angle_shift[2] = angle_shift[2] - swpAngle[1];
    angle_shift[0] = angle_shift[0] - swpAngle[2];
    
    // acceleration tare
    float* swpAcceleration = getAcceleration();
    acceleration_shift[0] = acceleration_shift[0] - swpAcceleration[0];
    acceleration_shift[1] = acceleration_shift[1] - swpAcceleration[1];
    acceleration_shift[2] = acceleration_shift[2] - swpAcceleration[2];

    // gyro tare
    float* swpGyro = getGyro();
    gyro_shift[0] = gyro_shift[0] - swpGyro[0];
    gyro_shift[1] = gyro_shift[1] - swpGyro[1];
    gyro_shift[2] = gyro_shift[2] - swpGyro[2];
    
    // magnetic field tare
    float* swpMagneticField = getMagneticField();
    magnetic_field_shift[0] = magnetic_field_shift[0] - swpMagneticField[0];
    magnetic_field_shift[1] = magnetic_field_shift[1] - swpMagneticField[1];
    magnetic_field_shift[2] = magnetic_field_shift[2] - swpMagneticField[2];
}

float* WitMotionInterface::getAcceleration() {
    float* calculated_acceleration = new float[3];
    
    calculated_acceleration[0] = acceleration[0] + acceleration_shift[0];
    calculated_acceleration[1] = acceleration[2] + acceleration_shift[1];
    calculated_acceleration[2] = acceleration[2] + acceleration_shift[2];
    
    return calculated_acceleration;
}

float* WitMotionInterface::getGyro() {
    float* calculated_gyro = new float[3];
    
    calculated_gyro[0] = gyro[0] + gyro_shift[0];
    calculated_gyro[1] = gyro[2] + gyro_shift[1];
    calculated_gyro[2] = gyro[2] + gyro_shift[2];
    
    return calculated_gyro;
}

float* WitMotionInterface::getAngle() {
    float* calculated_angle = new float[3];
        
    // mapping hardware x-y-z axis to openFrameworks x-y-z axis standarts
    calculated_angle[0] = angle[1] + angle_shift[1];
    calculated_angle[1] = angle[2] + angle_shift[2];
    calculated_angle[2] = angle[0] + angle_shift[0];
    
    return calculated_angle;
}

float* WitMotionInterface::getMagneticField() {
    float* calculated_magnetic_field = new float[3];
    
    calculated_magnetic_field[0] = magnetic_field[0] + magnetic_field_shift[0];
    calculated_magnetic_field[1] = magnetic_field[2] + magnetic_field_shift[1];
    calculated_magnetic_field[2] = magnetic_field[2] + magnetic_field_shift[2];
    
    return calculated_magnetic_field;
}

void WitMotionInterface::parseData(char chr) {
    static char chrBuf[100];
    static unsigned char chrCnt = 0;
    signed short sData[4];
    unsigned char i;
    char cTemp = 0;
    time_t now;
    chrBuf[chrCnt++] = chr;
    
    if (chrCnt < 11)
        return;
    for (i = 0; i < 10; i++)
    cTemp += chrBuf[i];
    if ((chrBuf[0] != 0x55) || ((chrBuf[1] & 0x50) != 0x50) || (cTemp != chrBuf[10])) {
        std::cout << "Error: " << chrBuf[0] << ", " << chrBuf[1] << std::endl;
        memcpy(&chrBuf[0], &chrBuf[1], 10);
        chrCnt--;
        return;
    }
    
    memcpy(&sData[0], &chrBuf[2], 8);
    switch (chrBuf[1]) {
        case 0x51:
            for (i = 0; i < 3; i++)
            acceleration[i] = (float)sData[i] / 32768.0 * 16.0;
            time(&now);
            break;
        case 0x52:
            for (i = 0; i < 3; i++)
            gyro[i] = (float)sData[i] / 32768.0 * 2000.0;
            break;
        case 0x53:
            for (i = 0; i < 3; i++)
            angle[i] = (float)sData[i] / 32768.0 * 180.0;
            break;
        case 0x54:
            for (i = 0; i < 3; i++)
            magnetic_field[i] = (float)sData[i];
            break;
    }
    chrCnt = 0;
}

void WitMotionInterface::calibrateDevice() {
    //TODO: Send three commands:
    // 1. “0xFF 0xAA 0x69 0x88 0xB5” for unblocking the config of the sensor
    // 2. “0xFF 0xAA 0x01 0x01 0x00” for calibration of acceleration
    // 3. “0xFF 0xAA 0x00 0x00 0x00” for saving the config (reference: changing the second to last byte to 0x01 sets to default settings)
}

void WitMotionInterface::setRefreshRate(char rate = 0x0b) {
    /*
     0xFF 0xAA 0x03 RATE 0x00
     0x01:0.2Hz
     0x02:0.5Hz
     0x03:1Hz
     0x04:2Hz
     0x05:5Hz
     0x06:10Hz(default)
     0x07:20Hz
     0x08:50Hz
     0x09:100Hz
     0x0a:125Hz
     0x0b:200Hz
     0x0c:Single
     0x0d: No output
     */
    char data[5] = { static_cast<char>(0xff), static_cast<char>(0xAA), 0x03, static_cast<char>(rate & 0xFF), static_cast<char>((rate>>8 & 0xFF)) };
    //serial.writeBytes(data, 5);
    
    //TODO: confirm by reading next bytes for:
    //0x55 0x5F 0xRATE
    
    char saveCommand[5] = { static_cast<char>(0xFF), static_cast<char>(0xAA), 0x00, 0x00, 0x00 };
    //serial.writeBytes(saveCommand, 5);
}

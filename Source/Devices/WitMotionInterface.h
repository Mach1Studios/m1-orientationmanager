//
//  m1-orientationmanager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>
#include <string>
#include <iostream>

#ifndef WIN32
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>
#endif

class WitMotionInterface {
public:
    WitMotionInterface();
    ~WitMotionInterface();
    
    // data receiving & parsing functions
    int recvData(char *recv_buffer, int length);
    void parseData(char chr);
    float* updateOrientation(char readBuffer[], int length);
    
    // get functions for data
    float* getAcceleration();
    float* getGyro();
    float* getAngle();
    float* getMagneticField();
    
    bool isConnected;
    int ret, fd;

    //acceleration
    float acceleration[3];
    //gyroscope
    float gyro[3];
    //angle
    float angle[3];
    //magnetic field
    float magnetic_field[3];

    //reset orientation
    float magnetic_field_shift[3];
    float acceleration_shift[3];
    float gyro_shift[3];
    float angle_shift[3];

    void resetOrientation();
    void recenter();
    void setRefreshRate(char rate);
    void calibrateDevice();
};

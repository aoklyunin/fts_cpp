//
// Created by alex on 10.01.18.
//

#ifndef FTS_CPP_DYNPICKER_H
#define FTS_CPP_DYNPICKER_H


#define DATA_LENGTH 30
#define CALIB_DATA_LENGTH 51

#include <zconf.h>
#include <chrono>
#include <string.h>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include <termio.h>

class DynPicker {
public:
    void connect(const char *comPort, bool auto_adjust);

    long readForces(bool flgZero, float *res);

    void closeConn();

    DynPicker();

private:
    int SetComAttr(int fdc);
    bool readFromCOM(const int length, char *reply);
    long getDeltaMicros();
    int64_t epoch_usec();
    int fdc;
    float *calib;
    float *offset;
    long prevTime;
};


#endif //FTS_CPP_DYNPICKER_H

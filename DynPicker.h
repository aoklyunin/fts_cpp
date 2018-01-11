//
// Created by alex on 10.01.18.
//

#ifndef FTS_CPP_DYNPICKER_H
#define FTS_CPP_DYNPICKER_H


#define DATA_LENGTH 30
#define CALIB_DATA_LENGTH 51

#include <zconf.h>
#include <chrono>

class DynPicker {
public:
    void connect(const char *comPort, bool auto_adjust);

    void readForces(bool flgZero, float *res);

    void closeConn();

    DynPicker();

private:
    int SetComAttr(int fdc);
    bool readFromCOM(const int length, char *reply);
    long getDeltaMicros();
    int fdc;
    float *calib;
    float *offset;
    long prevTime;
};


#endif //FTS_CPP_DYNPICKER_H

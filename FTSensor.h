//
// Created by alex on 10.01.18.
//

#ifndef FTS_CPP_DYNPICKER_H
#define FTS_CPP_DYNPICKER_H


#include <zconf.h>
#include <chrono>
#include <string.h>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include <termio.h>
#include "time.h"

class FTSensor {
public:
    void openConn(const char *comPort, bool flgNonBlocked);

    int readNext(float *res);

    int readLast(float *res);

    void closeConn();

    FTSensor(const char *comPort, bool non_blocked);

    int64_t get_delta_micros();

    ~FTSensor();

private:

    static const int _DATA_LENGTH = 30;
    static const int _CALIB_DATA_LENGTH = 51;
    static const int _buf_sz = 4096;
    int _buf_filled = 0;
    char _buf[_buf_sz];
    int _fdc;
    float _calib[6];
    int64_t _prev_time;
    bool _non_blocked;

    void _calibrate();

    int _set_com_attr(bool flgNonBlocked);

    int _getForceFromBuf(int start, float *res);

    int _read_from_COM(int length);

};


#endif //FTS_CPP_DYNPICKER_H

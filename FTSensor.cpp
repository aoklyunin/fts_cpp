//
// Created by alex on 10.01.18.
//

#include "FTSensor.h"


/*
 * read last package
 */
int FTSensor::readLast(float *res) {
    if (_buf_filled < _DATA_LENGTH)
        return -1;
    else
        return _getForceFromBuf(_buf_filled - _buf_filled % _DATA_LENGTH, res);
}


/*
 * get difference in microseconds between time of previous loop iteration and current
 */
int64_t FTSensor::get_delta_micros() {
    int64_t curns = epoch_usec();
    int64_t delta = (curns - _prev_time);
    _prev_time = curns;
    return delta;
}

/*
 * close connection
 */
void FTSensor::closeConn() {
    close(_fdc);
}

/*
 * open COMPort by it's address
 */
void FTSensor::openConn(const char *comPort, bool flgNonBlocked) {
    _fdc = open(comPort, O_RDWR);
    if (_fdc == -1) {
        throw std::runtime_error(std::string("Error in opening COM") + std::strerror(errno));
    }
    _set_com_attr(flgNonBlocked);
}

/*
 * get calibration coefficients of sensor
 */
void FTSensor::_calibrate() {
    // request to sensor for calibration coefficients
    write(_fdc, "p", 1);
    while (_buf_filled < _CALIB_DATA_LENGTH) {
        _read_from_COM(_CALIB_DATA_LENGTH);
    }

    // parse response string, first and last four symbols contain no data
    int ret = sscanf(std::string(_buf).substr(1, _CALIB_DATA_LENGTH - 4).c_str(),
                     "%f,%f,%f,%f,%f,%f",
                     &_calib[0], &_calib[1], &_calib[2], &_calib[3], &_calib[4], &_calib[5]);

    // if we can't response string
    if (ret == EOF) {
        throw std::runtime_error(std::string("Error in reading from COM") + std::strerror(errno));
    }

    // set end number to start position
    _buf_filled = 0;
}

/*
 * read next force
 */
int FTSensor::readNext(float *res) {
    // request to sensor to get torques
    write(_fdc, "R", 1);
    int c = _read_from_COM(_DATA_LENGTH);
    if (c == -1)
        return -1;
    if (_buf_filled < _DATA_LENGTH)
        return 0;

    FTSensor::_getForceFromBuf(0, res);
    return 1;
}

/*
 * get force from target buf position
 * and remove all symbols to the end of current package
 */

int FTSensor::_getForceFromBuf(int start, float *res) {

    // parse response string
    for (unsigned long i = 0; i < 6; i++) {
        // parse i-th value from string
        // (each of them takes 2 bites or 4 symbols)

        short val = 0;

        int ret = sscanf(std::string(_buf).substr(i * 4 + 3, i * 4 + 7).c_str(), "%4hx", &val);

        //if we get error by parsing string
        if (ret == EOF) {
            throw std::runtime_error(std::string("Error in reading from COM") + std::strerror(errno));
        }
        // get signed value
        res[i] = static_cast<float>(val - 8192) / _calib[i];
    }
    std::memmove(_buf + _DATA_LENGTH + start, _buf, static_cast<size_t>(_buf_sz - (_DATA_LENGTH + start)));
    _buf_filled -= _DATA_LENGTH;
    return 1;
}

/*
 * set com port params
 */
int FTSensor::_set_com_attr(bool flgNonBlocked) {
    int n;

    struct termios term;

    // Set baud rate
    n = tcgetattr(_fdc, &term);
    if (n < 0)
        goto over;

    bzero(&term, sizeof(term));

    term.c_cflag = B921600 | CS8 | CLOCAL | CREAD;
    term.c_iflag = IGNPAR;
    term.c_oflag = 0;
    term.c_lflag = 0;/*ICANON;*/

    term.c_cc[VINTR] = 0;     /* Ctrl-c */
    term.c_cc[VQUIT] = 0;     /* Ctrl-? */
    term.c_cc[VERASE] = 0;     /* del */
    term.c_cc[VKILL] = 0;     /* @ */
    term.c_cc[VEOF] = 4;     /* Ctrl-d */
    term.c_cc[VTIME] = 0;
    term.c_cc[VMIN] = 0;
    term.c_cc[VSWTC] = 0;     /* '?0' */
    term.c_cc[VSTART] = 0;     /* Ctrl-q */
    term.c_cc[VSTOP] = 0;     /* Ctrl-s */
    term.c_cc[VSUSP] = 0;     /* Ctrl-z */
    term.c_cc[VEOL] = 0;     /* '?0' */
    term.c_cc[VREPRINT] = 0;     /* Ctrl-r */
    term.c_cc[VDISCARD] = 0;     /* Ctrl-u */
    term.c_cc[VWERASE] = 0;     /* Ctrl-w */
    term.c_cc[VLNEXT] = 0;     /* Ctrl-v */
    term.c_cc[VEOL2] = 0;     /* '?0' */

    if (flgNonBlocked) {
        const int flags = fcntl(_fdc, F_GETFL, 0);
        fcntl(_fdc, F_SETFL, flags | O_NONBLOCK);
    }
//  tcflush(fdc, TCIFLUSH);
    n = tcsetattr(_fdc, TCSANOW, &term);
    over :
    return (n);
}

/*
 * read bytes from com
 */
int FTSensor::_read_from_COM(const int length) {
    ssize_t numBytes;
    if (_non_blocked) {
        // check if _fdc has new bytes
        if ((numBytes = read(_fdc, _buf + _buf_filled, (size_t) length)) < 0) {
            if (errno != EWOULDBLOCK) {
                // TODO: обработать ошибку ввода-вывода
                perror("read");
                exit(EXIT_FAILURE);
            }else{
                return -1;
            }
        } else {
            // Как-либо обрабатываем данные.
            _buf_filled += numBytes;
        }
    } else {
        if ((numBytes = read(_fdc, _buf + _buf_filled, (size_t) length)) >= 0) {
            _buf_filled += numBytes;
        }
    }
    return static_cast<int>(numBytes);
}

/*
 * Constructor
 */
FTSensor::FTSensor(const char *comPort, bool non_blocked) {
    _non_blocked = non_blocked;
    _prev_time = epoch_usec();
    for (float &i : _calib)
        i = 1;
    openConn(comPort, non_blocked);
    _calibrate();
}

/*
 * Destructor
 */
FTSensor::~FTSensor() {
    closeConn();
}

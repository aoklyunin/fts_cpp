//
// Created by alex on 10.01.18.
//

#include <cstdio>
#include <fcntl.h>
#include <termio.h>
#include "DynPicker.h"
#include <string.h>
#include <iostream>
#include <cstring>

/*
 * получить разницу в микросекундах
 */
long DynPicker::getDeltaMicros() {
    struct timespec curTime;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &curTime);
    long curns = curTime.tv_nsec;
    long delta = (curns - prevTime) / 1000;
    prevTime = curns;
    return delta;
}

/*
 * закрыть соединение
 */
void DynPicker::closeConn() {
    close(fdc);
}

/*
 * подключение
 * comPort - строка с адресом порта
 * auto_adjust - нужно ли прочитать калибровочные коэффициенты из датчика
 */
void DynPicker::connect(const char *comPort, bool auto_adjust) {
    fdc = open(comPort, O_RDWR);
    if (fdc == -1) {
        throw std::runtime_error(std::string("Error in opening COM") + std::strerror(errno));
    }
    SetComAttr(fdc);
    calib = new float[6]{1, 1, 1, 1, 1, 1};
    if (auto_adjust) {
        // запрос к датчику, чтобы получить измерения
        write(fdc, "p", 1);
        // читаем ответ
        char reply[CALIB_DATA_LENGTH];
        readFromCOM(CALIB_DATA_LENGTH, reply);
        // парсим строку с ответом
        int ret = sscanf(std::string(reply).substr(1, int(CALIB_DATA_LENGTH) - 4).c_str(),
                         "%f,%f,%f,%f,%f,%f",
                         &calib[0], &calib[1], &calib[2], &calib[3], &calib[4], &calib[5]);
        // если не получилось распарсить
        if (ret == EOF) {
            throw std::runtime_error(std::string("Error in reading from COM") + std::strerror(errno));
        }
    }
    offset = new float[6];
    readForces(false, offset);
}

/*
 * подключение
 * flgZero - флаг, нужно ли вычитать из показаний датчика показания, полученные при первом измерении
 * res - указатель на массив, в который записываются показания датчиков
 */
void DynPicker::readForces(bool flgZero, float *res) { // Request for initial single data
    // запрос к датчику, чтобы получить измерения
    write(fdc, "R", 1);
    // читаем ответ
    char reply[DATA_LENGTH];
    readFromCOM(DATA_LENGTH, reply);
    // парсим строку с ответом
    for (int i = 0; i < 6; i++) {
        // парсим i-е значение из строки ( каждое из них занимает 2 байта или 4 символа)
        unsigned short val = 0;
        int ret = sscanf(std::string(reply).substr(i * 4 + 3, i * 4 + 7).c_str(), "%4hx", &val);
        // если не получилось распарсить
        if (ret == EOF) {
            throw std::runtime_error(std::string("Error in reading from COM") + std::strerror(errno));
        }
        // т.к. у нас переменная знаковая, то нужно вычесть константу для получения показаний датчика
        // в нормальной форме, также полученное значение необходимо разделить на калибровочную константу
        res[i] = float((val - 8192)) / calib[i];
        if (flgZero) res[i] -= offset[i];
    }
    // выводим кол-во микросекунд на одну итерацию
    std::cout << getDeltaMicros() << std::endl;
}

/*
 * задаём параметры ком-порта
 */
int DynPicker::SetComAttr(int fdc) {
    int n;

    struct termios term;


    // Set baud rate
    n = tcgetattr(fdc, &term);
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

//  tcflush(fdc, TCIFLUSH);
    n = tcsetattr(fdc, TCSANOW, &term);
    over :
    return (n);
}

/*
 * читает байты из ком-порта с ожиданием, пока не будет считано нужное число байт
 * length - необходимое кол-во байт
 * reply - буффер, в который будет произведено чтение
 */
bool DynPicker::readFromCOM(const int length, char *reply) {
    int len = 0;
    while (len < length) {
        int c = read(fdc, reply + len, length - len);
        if (c >= 0) {
            len += c;
        } else {
            throw std::runtime_error(std::string("Error in reading from COM") + std::strerror(errno));
        }
    }
    return true;
}

/*
 * Конструктор
 */
DynPicker::DynPicker() {
    struct timespec curTime;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &curTime);
    prevTime = curTime.tv_nsec;
}

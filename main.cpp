#include <iostream>
#include "FTSensor.h"
#include <fstream>


// время эксперимента
#define measCnt 6000000

int main() {
    FTSensor dp("/dev/ttyUSB0",true);
    std::ofstream myfile;
    myfile.open("../log/log.csv");

    for (int i = 0; i < measCnt; i++) {
        float res[6];
        int dt = dp.readNext(res);
        if (dt == 1) {
            char buffer[100];
            sprintf(buffer, "%d, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f",
                    dt, res[0], res[1], res[2], res[3], res[4],
                    res[5]);
            myfile << buffer << std::endl;
            // std::cout <<  res[2] << std::endl;
            sprintf(buffer, "%4li %.1f  %.2f\n",dp.get_delta_micros(), res[2], res[3]);
            std::cout << buffer;
        }
    }
    std::cout << "end" << std::endl;
    myfile.close();
    return 0;
}
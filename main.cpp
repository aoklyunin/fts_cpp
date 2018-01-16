#include <iostream>
#include "DynPicker.h"
#include <fstream>


// время эксперимента
#define measCnt 30000

int main() {
    DynPicker dp;
    dp.connect("/dev/ttyUSB0", true);
    std::ofstream myfile;
    myfile.open("../log/log.csv");

    for (int i = 0; i < measCnt; i++) {
        float res[6];
        long dt = dp.readForces(false, res);
        char buffer[100];
        sprintf(buffer, "%d, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f", dt, res[0], res[1], res[2], res[3], res[4], res[5]);
        myfile << buffer << std::endl;
       // std::cout <<  res[2] << std::endl;
        sprintf(buffer,"%.1f\%\n",(float)i/measCnt*100);
        std::cout<<buffer;
    }
    std::cout << "end" << std::endl;
    dp.closeConn();
    myfile.close();
    return 0;
}
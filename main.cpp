#include <iostream>
#include "DynPicker.h"

int main() {
    DynPicker dp;
    dp.connect("/dev/ttyUSB0", true);

    for (int i=0;i<10000;i++) {
        float res[6];
        dp.readForces(true,res);
        char buffer [100];
        sprintf(buffer, "%6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f", res[0], res[1], res[2], res[3], res[4], res[5]);
        std::cout << buffer << std::endl;
    }
    std::cout<<"end"<<std::endl;
    dp.closeConn();
    return 0;
}
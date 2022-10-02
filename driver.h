#ifndef driver_h
#define driver_h

#include "Arduino.h"

class driver{

    private:
        int enPin;
        int clkPin;
        int aclkPin;
        int pwmPin;
        int csPin;
        char dir;

        float curr;

    public:
        driver(int p1, int p2, int p3, int p4, int p5);
        void init();
        void setDir(char d);
        void drive(int duty);
        float getCurr();

};

#endif

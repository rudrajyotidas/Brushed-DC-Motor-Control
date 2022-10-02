#include "driver.h"
#include "Arduino.h"

driver::driver(int p1, int p2, int p3, int p4, int p5){
    enPin = p1;
    clkPin = p2;
    aclkPin = p3;
    pwmPin = p4;
    csPin = p5;
}

void driver::init(){

    pinMode(enPin, OUTPUT);
    pinMode(clkPin, OUTPUT);
    pinMode(aclkPin, OUTPUT);
    pinMode(pwmPin, OUTPUT);
    pinMode(csPin, INPUT);

    digitalWrite(enPin, HIGH);
    digitalWrite(clkPin, LOW);
    digitalWrite(aclkPin, HIGH);
    dir = 'a';
    digitalWrite(pwmPin, LOW);
}

void driver::drive(int duty){
    analogWrite(pwmPin, duty);
}

void driver::setDir(char d){
    if(d == 'a'){
        digitalWrite(clkPin, LOW);
        digitalWrite(aclkPin, HIGH);

        dir = 'a';
    }

    else if(d == 'c'){
        digitalWrite(aclkPin, LOW);
        digitalWrite(clkPin, HIGH);

        dir = 'c';
    }
}

float driver::getCurr(){
    
    int s = analogRead(csPin);
    float curr = (float)5*0.3*11370*(((float)s/1024)/1500);

    return curr;
}
#include "encoder.h"
#include "Arduino.h"

#include <Wire.h>

encoder::encoder(){
    
    magStatus = 0;
    rawLOW = 0; 
    rawHIGH = 0; 
    rawAngle = 0;
    degAngle = 0; 
    startAngle = 0; 
    correctedAngle = 0; 
    totalAngle = 0;

    quadrant = 0; 
    prev_quadrant = 0; 
    num_turns = 0;
}

void encoder::checkMagnet(){

    while(magStatus&(1<<5) == 0){

        magStatus = 0;

        Wire.beginTransmission(0x36);
        Wire.write(0x0B);
        Wire.endTransmission();
        Wire.requestFrom(0x36, 1);

        while(Wire.available() == 0);
        magStatus = Wire.read();
    }
}

void encoder::readAngle(){

    Wire.beginTransmission(0x36);
    Wire.write(0x0D);
    Wire.endTransmission();
    Wire.requestFrom(0x36, 1);

    while(Wire.available() == 0);
    rawLOW = Wire.read();


    Wire.beginTransmission(0x36);
    Wire.write(0x0C);
    Wire.endTransmission();
    Wire.requestFrom(0x36, 1);

    while(Wire.available() == 0);
    rawHIGH = Wire.read();
    rawHIGH = rawHIGH<<8;


    rawAngle = rawHIGH|rawLOW;

    degAngle = rawAngle*(360.0/4095);
}

void encoder::correctAngle(){

    correctedAngle = degAngle - startAngle;

    if(correctedAngle<0) correctedAngle += 360;
}

void encoder::checkQuadrantUpdateTurns(){

    //Quadrant 1
    if(correctedAngle >= 0 && correctedAngle <=90) quadrant=1;

    //Quadrant 2
    if(correctedAngle > 90 && correctedAngle <=180) quadrant=2;

    //Quadrant 3
    if(correctedAngle > 180 && correctedAngle <=270) quadrant=3;

    //Quadrant 4
    if(correctedAngle > 270 && correctedAngle <360) quadrant=4;


    /*Update Turns*/
    if(quadrant != prev_quadrant){

        if(quadrant==1 && prev_quadrant==4) num_turns++;
        else if(quadrant==4 && prev_quadrant==1) num_turns--;
    }

    prev_quadrant = quadrant;

    totalAngle_prev = totalAngle;
    totalAngle = correctedAngle + num_turns*360;
}

void encoder::init(){

    checkMagnet();
    readAngle();

    startAngle = degAngle;
    t1 = t2 = micros()/1e6;
}

float encoder::getAngle(){

    t1 = t2;
    t2 = micros()/1e6;

    readAngle();
    correctAngle();
    checkQuadrantUpdateTurns();

    return totalAngle;
}

float encoder::getVel(){

    return (totalAngle - totalAngle_prev)/(t2 - t1);
}

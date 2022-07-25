#include<Wire.h>

char magStatus = 0;

/*
 * rawLOW : Lower 8 bits of RAW ANGLE register
 * rawHIGH : Upper 4 bits of RAW ANGLE register
 * rawANGLE: Total 12 bit raw angle
 * degAngle: Angle in degrees. degAngle = (360/4095)*rawAngle
 * startAngle: Angle at the beginning
 * correctedAngle: Angle with respect to start angle. correctecAngle = degAngle - startAngle
 * totalAngle: Angle taking into account rotations. totalAngle = num_turns*360 + correctedAngle
 */

int rawLOW=0; 
int rawHIGH=0; 
int rawAngle=0;
float degAngle=0; 
float startAngle=0; 
float correctedAngle=0; 
float totalAngle=0;

int quadrant = 0; 
int prev_quadrant = 0; 
int num_turns=0;

void setup(){

    Serial.begin(9600);
    Wire.begin();
    Wire.setClock(800000L);

    checkMagnet();

    readAngle();
    startAngle = degAngle;
}

void loop(){

    readAngle();
    correctAngle();
    checkQuadrantUpdateTurns();

    Serial.print("Rotation: ");
    Serial.print(totalAngle);
    Serial.print("\r\n");
}

void checkMagnet(){

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

void readAngle(){

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

void correctAngle(){

    correctedAngle = degAngle - startAngle;

    if(correctedAngle<0) correctedAngle += 360;
}

void checkQuadrantUpdateTurns(){

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

    totalAngle = correctedAngle + num_turns*360;
}

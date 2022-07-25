#include<Wire.h>

#define ENABLE_1 A0
#define ENABLE_2 A1

#define CW_MOTOR_1 7
#define ACW_MOTOR_1 8
#define CW_MOTOR_2 4
#define ACW_MOTOR_2 9

#define PWM_1 5
#define PWM_2 6

#define CURRENT_SENSE_1 A2
#define CURRENT_SENSE_2 A3

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
float totalAngle_0=0;

float totalAngle_1=0;
float filteredAngle_0=0;
float filteredAngle_1=0;

float vel_0 = 0;
float vel_1 = 0;
float curr_time = 0;
float prev_time = 0;
float filtered_vel_0 = 0;
float filtered_vel_1 = 0;

int quadrant = 0; 
int prev_quadrant = 0; 
long long num_turns=0;

int rpm_ref = 4000;
int angle_ref = 7200;

int maxAngle = 11000;

float position_error = 0;
//float posiion_error_prev = 0;
float Kp_position = 20000;
float Kd_position = 0.465;
float control_PK = 0;
float control_PD = 0;

int duty_cycle=0;
float Vcc = 12;
float Kp = 30;
float Kd = 8;
float Ki = 20;
float controlKp = 0;
float controlKd = 0;
float controlKi = 0;
float error=0;
//float prev_error=0;

void setup(){

    Serial.begin(230400);
    Wire.begin();
    //Wire.setClock(800000L);

    checkMagnet();

    readAngle();
    startAngle = degAngle;

    pinMode(ENABLE_1, OUTPUT);
    pinMode(ENABLE_2, OUTPUT);
  
    pinMode(ACW_MOTOR_1, OUTPUT);
    pinMode(CW_MOTOR_1, OUTPUT);
    pinMode(ACW_MOTOR_2, OUTPUT);
    pinMode(CW_MOTOR_2, OUTPUT);

    pinMode(PWM_1, OUTPUT);
    pinMode(PWM_2, OUTPUT);

    pinMode(CURRENT_SENSE_1, INPUT);
    pinMode(CURRENT_SENSE_2, INPUT);

    digitalWrite(ENABLE_2, HIGH);

    digitalWrite(CW_MOTOR_2, HIGH);
    digitalWrite(ACW_MOTOR_2, LOW);

    //analogWrite(PWM_2, 50);
}

void loop(){

    readAngle();
    correctAngle();
    checkQuadrantUpdateTurns();

//    position_error = (angle_ref - filteredAngle_0);
//    control_PK = position_error*Kp_position;
//    control_PD = Kd_position*(filteredAngle_1 - filteredAngle_0)/(curr_time - prev_time); 
//    rpm_ref = control_PK + control_PD;
    
    applyFilterUpdateControl();

//    error = rpm_ref - filtered_vel_0;
//
//    controlK = error*Kp;
//    controlI = error*(curr_time - prev_time)*Ki;

    

    float desired_voltage = controlKd + controlKp + controlKi;

    duty_cycle = (desired_voltage/Vcc)*255;

    drive(duty_cycle);

    Serial.print(0);
    Serial.print(" ");
    Serial.print(angle_ref);
    Serial.print(" ");
    Serial.print("RPM: ");
    Serial.println(filteredAngle_0);
    //Serial.print("\r\n");
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

    totalAngle_0 = correctedAngle + num_turns*360;
}

void applyFilterUpdateControl(){
    filteredAngle_0 = 0.969*filteredAngle_1 + 0.0155*totalAngle_0 + 0.0155*totalAngle_1;
    //vel_0 = 0.9937*vel_1 + 30.93*(totalAngle_0 - totalAngle_1); 

    curr_time = micros()/1e6;

    position_error = (angle_ref - filteredAngle_0)/(float)maxAngle;
    control_PK = position_error*Kp_position;
    control_PD = Kd_position*(filteredAngle_1 - filteredAngle_0)/(curr_time - prev_time); 
    rpm_ref = control_PK + control_PD;

    
    vel_0 = 60.0*(filteredAngle_0 - filteredAngle_1)/((curr_time - prev_time)*360);
    filtered_vel_0 = 0.9875*filtered_vel_1 + 0.00624*vel_0 + 0.00624*vel_1;

    error = (rpm_ref - filtered_vel_0)/4500.0;

    controlKp = error*Kp;
    controlKd = (filtered_vel_1 - filtered_vel_0)*Kd/((curr_time - prev_time)*4500);
    controlKi += error*Ki*(curr_time - prev_time);

    if(controlKi>Vcc) controlKi = 4;

    filteredAngle_1 = filteredAngle_0;
    totalAngle_1 = totalAngle_0;
    vel_1 = vel_0;
    filtered_vel_1 = filtered_vel_0;
    prev_time = curr_time;
    //prev_error = error;
}

void  drive(int duty_cycle){
  
  if(duty_cycle > 255){
    analogWrite(PWM_2, 255);
  }

  else if(duty_cycle < 0){
    analogWrite(PWM_2, 0);
  }

  else{
    analogWrite(PWM_2, duty_cycle);
  }
}

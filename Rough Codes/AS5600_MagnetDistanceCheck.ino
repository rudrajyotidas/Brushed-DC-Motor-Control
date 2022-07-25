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

void setup(){

    
  Serial.begin(9600);

  Wire.begin();
  Wire.setClock(800000L);

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

  analogWrite(PWM_2, 100);

    //int detected = 0, high = 0, low = 0;

}

void loop(){

    char c;

    Wire.beginTransmission(0x36);
    Wire.write(0x0B);
    Wire.endTransmission();
    Wire.requestFrom(0x36, 1);

    while(Wire.available() == 0);
    c = Wire.read();

    Serial.print("HIGH:");

    if(c&(1<<3)) Serial.print(1);
    else Serial.print(0);

    Serial.print("   LOW:");

    if(c&(1<<4)) Serial.print(1);
    else Serial.print(0);

    Serial.print("   DETECTED:");

    if(c&(1<<5)) Serial.print(1);
    else Serial.print(0);

    Wire.beginTransmission(0x36);
    Wire.write(0x0D);
    Wire.endTransmission();
    Wire.requestFrom(0x36, 1);

    while(Wire.available() == 0);
    int rawLOW = Wire.read();

    Serial.print("   RAW LOW:");
    Serial.print(rawLOW);

    Wire.beginTransmission(0x36);
    Wire.write(0x0C);
    Wire.endTransmission();
    Wire.requestFrom(0x36, 1);

    while(Wire.available() == 0);
    int rawHIGH = Wire.read();

    rawHIGH = rawHIGH<<8;

    Serial.print("   RAW HIGH:");
    Serial.print(rawHIGH);


    int rawAngle = rawHIGH|rawLOW;

    Serial.print("   RAW ANGLE:");
    Serial.print(rawAngle);

    float degAngle = rawAngle*(360.0/4095);

    Serial.print("   ANGLE:");
    Serial.print(degAngle);

    Serial.print("\n");

    //delay(1000);
}

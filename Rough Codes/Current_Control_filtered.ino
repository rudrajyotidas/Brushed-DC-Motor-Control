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

float curr_0 = 0;
float curr_1 = 0;
float filtered_0 = 0;
float filtered_1 = 0;

float Kp = 20;
float Ki = 20;
float curr_ref = 0.9;
float curr_time = 0;
float prev_time = 0;
float error = 0;
float controlKp = 0;
float controlKi = 0;

int Vcc = 12;
int duty_cycle = 0;

void setup() {

  Serial.begin(9600);
  
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

  analogWrite(PWM_2, 0);
}

void loop() {

  /* -------- READ CURRENT AND APPLY LOW PASS FILTER -----------*/
  int s = analogRead(A3);
  curr_0 = (float)5*0.3*11370*(((float)s/1024)/1500);

  filtered_0 = 0.969*filtered_1 + 0.0155*curr_0 + 0.0155*curr_1;

  curr_1 = curr_0;
  filtered_1 = filtered_0;

  /* -------- FIND ERROR AND DUTY CYCLE ----------*/

  curr_time = micros()/1e6;

  error = curr_ref - filtered_0;
  controlKp = error*Kp;
  controlKi = controlKi + (curr_time - prev_time)*error*Ki;

  //windup
  if(controlKi >= Vcc/3) controlKi = 2;
  
  float desired_voltage = controlKp + controlKi;
  duty_cycle = (desired_voltage/Vcc)*255;

  /* ----------- APPLY DUTY CYCLE -----------*/

  if(duty_cycle>255) analogWrite(PWM_2, 255);
  else if(duty_cycle<0) analogWrite(PWM_2, 0);
  else analogWrite(PWM_2, duty_cycle);

  //time update
  prev_time = curr_time;

  Serial.println(filtered_0);
 }

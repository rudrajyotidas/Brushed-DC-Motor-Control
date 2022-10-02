#include "motorcontrol.h"
#include "Arduino.h"

motorController::motorController(int p1, int p2, int p3, int p4, int p5):d(p1, p2, p3, p4, p5){

    Vcc = 12;
    maxRPM = 4500;
    maxAngle = 110000;

    Kp_curr = 30;
    Ki_curr = 40;

    Kp_vel = 60;
    Ki_vel = 30;
    Kd_vel = 0;

    Kp_pos = 4000;
    Ki_pos = 0;
    Kd_pos = 0.0031;

    current_0 = 0;
    current_1 = 0;
    velocity_0 = 0;
    velocity_1 = 0;
    position_0 = 0;
    position_1 = 0;

    filtered_curr_1 = 0;
    filtered_curr_0 = 0;
    filtered_vel_1 = 0;
    filtered_vel_0 = 0;
    filtered_pos_1 = 0;
    filtered_pos_0 = 0;
}

void motorController::filter_current(){
    filtered_curr_1 = filtered_curr_0;
    filtered_curr_0 = 0.969*filtered_curr_1 + 0.0155*current_0 + 0.0155*current_1;
}

void motorController::filter_velocity(){
    filtered_vel_1 = filtered_vel_0;
    filtered_vel_0 = 0.9875*filtered_vel_1 + 0.00624*velocity_0 + 0.00624*velocity_1;
}

void motorController::filter_position(){
    filtered_pos_1 = filtered_pos_0;
    filtered_pos_0 = 0.969*filtered_pos_1 + 0.0155*position_0 + 0.0155*position_1;
}

void motorController::refresh(){
    current_0 = 0;
    current_1 = 0;
    velocity_0 = 0;
    velocity_1 = 0;
    position_0 = 0;
    position_1 = 0;

    filtered_curr_1 = 0;
    filtered_curr_0 = 0;
    filtered_vel_1 = 0;
    filtered_vel_0 = 0;
    filtered_pos_1 = 0;
    filtered_pos_0 = 0;
}

void motorController::init(){
    refresh();
    d.init();
    e.init();
}

void motorController::updateState(){

    current_1 = current_0;
    velocity_1 = velocity_0;
    position_1 = position_0;

    current_0 = d.getCurr();
    position_0 = e.getAngle();
    velocity_0 = e.getVel();

    filter_current();
    filter_velocity();
    filter_position();
}

void motorController::seek_current(float curr_ref, char dir, bool& exit_variable){

    /*Set Direction of Motor*/
    d.setDir(dir);

    /*Refresh all state variables to 0*/
    refresh();

    /*Initialise Variables*/
    float curr_time = micros()/1e6;
    float prev_time = curr_time;

    float error = 0;
    float controlKp = 0;
    float controlKi = 0;
    int duty_cycle = 0;

    while(!exit_variable){

        /*Find current time and update variables*/
        curr_time = micros()/1e6;
        updateState();

        /*Find error and evaluate Kp, Ki, Kd*/
        error = curr_ref - filtered_curr_0;
        controlKp = Kp_curr*error;
        controlKi += Ki_curr*(error)*(curr_time - prev_time);

        /*Anti-Windup*/ 
        if(controlKi >= Vcc/1.5) controlKi = 2;

        /*Find required duty cycle and apply*/
        float control = controlKp + controlKi;
        duty_cycle = (control/Vcc)*255;

        if(duty_cycle > 255) d.drive(255);
        else if(duty_cycle < 0) d.drive(0);
        else d.drive(duty_cycle);

        /*Update time*/
        prev_time = curr_time;
    }
}

void motorController::seek_velocity(float vel_ref, bool& exit_variable){

    //input in rpm
    /*Set direction according to sign of velocity input*/
    if(vel_ref < 0){
        d.setDir('c');
    }
    else d.setDir('a');

    /*Refresh all variables to 0*/
    refresh();

    float curr_time = micros()/1e6;
    float prev_time = curr_time;
    float error = 0;
    float controlKp = 0;
    float controlKi = 0;
    float controlKd = 0;
    int duty_cycle = 0;

    while(!exit_variable){
        /*Find current time and update state variables*/
        curr_time = micros()/1e6;
        updateState();

        /*Find error and evaluate Kp, Ki, Kd after converting sensor data to rpm*/
        float curr_rpm = filtered_vel_0/6.0;
        error = (vel_ref - curr_rpm)/maxRPM;  //Normalise error

        controlKp = error*Kp_vel;
        controlKi += error*Ki_vel*(curr_time - prev_time);
        controlKd = Kd_vel*(filtered_vel_1 - filtered_vel_0)/((curr_time - prev_time)*maxRPM);

        Serial.print(" ");
        Serial.print(controlKp);
        Serial.print(" ");
        Serial.print(controlKi);
        Serial.print(" ");
        Serial.print(controlKd);

        /*Anti-Windup*/
        if(controlKi > Vcc){
            controlKi = 3;
        }
        else if(controlKi < -Vcc){
            controlKi = -4;
        }
        

        /*Evaluate duty cycle and apply control, with proper sign handling*/
        duty_cycle = (controlKp + controlKi + controlKd)*255/Vcc;

        Serial.print("  ");
        Serial.println(duty_cycle);

        if(vel_ref>0){
            if(duty_cycle > 255) d.drive(255);
            else if(duty_cycle < 0) d.drive(0);
            else d.drive(duty_cycle);
        }
        else if(vel_ref<0){
            if(duty_cycle < -255) d.drive(255);
            else if(duty_cycle > 0) d.drive(0);
            else d.drive(-duty_cycle);
        }

        prev_time = curr_time;
    }
}

void motorController::seek_position(float pos_ref, int max_duty_cycle, bool& exit_variable){

    refresh();
    updateState();

    if(pos_ref - position_0>0){
        d.setDir('a');
    }
    else{
        d.setDir('c');
    }

    float curr_time = micros()/1e6;
    float prev_time = curr_time;
    float error_P = 0;
    float controlKp_P = 0;
    float controlKi_P = 0; 
    float controlKd_P = 0;
    int duty_cycle = 0;

    while(!exit_variable){

        updateState();
        curr_time = micros()/1e6;

        error_P = (pos_ref - filtered_pos_0)/maxAngle;

        controlKp_P = Kp_pos*(error_P);
        controlKi_P += Ki_pos*(error_P)*(curr_time-prev_time);
        controlKd_P = Kd_pos*(filtered_pos_1-filtered_pos_0)/(curr_time - prev_time);

        if(controlKi_P>Vcc/2){
            controlKi_P = 2;
        }

        if(abs(error_P*maxAngle) > 300 && abs(error_P*maxAngle) < 100){
            controlKi_P = controlKi_P/30;
        }

        duty_cycle = 255*(controlKp_P + controlKi_P + controlKd_P)/Vcc;

        if(duty_cycle<0){
            d.setDir('c');
            duty_cycle *= -1;
        }
        else{
            d.setDir('a');
        }

        if(duty_cycle>max_duty_cycle){
            d.drive(max_duty_cycle);
        }
        else if(duty_cycle<0){
            d.drive(0);
        }
        else{
            d.drive(duty_cycle);
        }

        prev_time = curr_time;
    }


}

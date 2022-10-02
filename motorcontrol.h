#include "driver.h"
#include "encoder.h"

#ifndef motorcontrol_h
#define motorcontrol_h

class motorController{

    private:
        driver d;
        encoder e;
        
        float Vcc;
        float maxRPM;
        float maxAngle;

        float current_0;
        float current_1;
        float velocity_0;
        float velocity_1;
        float position_0;
        float position_1;

        float filtered_curr_1;
        float filtered_curr_0;
        float filtered_vel_1;
        float filtered_vel_0;
        float filtered_pos_1;
        float filtered_pos_0;

        float Kp_curr;
        float Ki_curr;

        float Kp_vel;
        float Ki_vel;
        float Kd_vel;

        float Kp_pos;
        float Ki_pos;
        float Kd_pos;

        

    public:

        motorController(int p1, int p2, int p3, int p4, int p5);

        void filter_current();
        void filter_velocity();
        void filter_position();
        void refresh();

        void init();

        void updateState();
        void seek_current(float curr_ref, char dir);
        void seek_velocity(float vel_ref);
        void seek_position(float pos_ref, int max_duty_cycle, bool& exit_variable);

};

#endif

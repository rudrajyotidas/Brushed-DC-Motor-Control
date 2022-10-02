#ifndef encoder_h
#define encoder_h

#include "Arduino.h"

class encoder{

    private:
        
        char magStatus;
        int rawLOW; 
        int rawHIGH; 
        int rawAngle;
        float degAngle; 
        float startAngle; 
        float correctedAngle; 
        double totalAngle;
        float totalAngle_prev;

        float t1;
        float t2;

        int quadrant; 
        int prev_quadrant; 
        double num_turns;

    public:
        encoder();
        void checkMagnet();
        void readAngle();
        void correctAngle();
        void checkQuadrantUpdateTurns();
        void init();
        float getAngle();
        float getVel();
};

#endif

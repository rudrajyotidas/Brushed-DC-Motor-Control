# How to Use the Library

This is a modular motor control library, created exclusively for DC Motors along with a magnetic encoder. Currently, this library supports only the AS5600 magnetic encoder and can only handle a single I2C device of the same address. However any motor driver board that works similar to the Sparkfun Monster Motor Shield can be used (need not be a dual motor driver).

There are three header files included, "driver.h", "encoder.h", and "motorcontrol.h". The latter depends on the former two.

## Encoder Header File

The encoder header file contains the following utillities
* `void checkMagnet()` contains an infinite loop that does not let the program proceed unless a magnet has been detected by the encoder module

* `void init()` initialises the variables and records the first raw reading given by the encoder. All angles are measured **relative to the starting angle**

* `float getAngle()` returns the angle rotated by the magnet relative to the starting orientation in degrees. The angle is signed, and if you want to reverse the sign of the angle readings, swap the DIR pin from GND to VCC or vice versa.

* `getVel()` returns a velocity in terms of degrees/second reading by differentiating the angles.

## Motor Driver Header File

To initialise the driver, write\
`driver objectname(enPin, clkPin, aclkPin, pwmPin, csPin)`

**enPin** enables the motor driver (or the particular channel of a dual motor driver)

**clkPin** and **aclkPin** together control the direction of the motor. Setting clkPin HIGH and aclkPin LOW makes the motor rotate in one direction while doing the opposite makes the motor rotate in the other direction

**pwmPin** receives the PWM signal to drive the motor

**csPin** is the current sense pin

The driver header file has the following utilities:
* `void init()` initilialises the driver by configuring the required pins

* `void setDir(char dir)` takes an argument 'a' or 'c' and accordingly sets the rotation direction clockwise or anticlockwise

* `void drive(int duty_cycle)` drives the motor according to the provided duty cycle value (in between 0 and 255) 

* `float getCurr()` fetches the current reading from the **csPin** of the driver

## Motor Control Header File

In order to use this header for motor control, first make sure you have a compatible driver and encoder. Then, just `#include "motorcontrol.h"` and initialise a **motorController** object as

`motorController objectname(enPin, clkPin, aclkPin, pwmPin, csPin)`

This object has the following public member functions:

* `void init()` initialises the **motorController** object

* `void seek_current(float max_curr, char dir, bool& exit_variable)` is the function you need to call for current control. The motor will never draw a current more than **max_curr** while rotating in the direction specified via **dir** (either 'a' or 'c'). The third argument is a boolean variable that you need to *pass by reference*. The function stops and exits when the boolean variable is true. You need to manipulate this variable using interrupts

* `void seek_velocity(float target_rpm, bool& exit_variable)` is the function to call for velocity control. The motor will rotate at a constant rpm, as passed through the argument. The other argument is a boolean variable that you need to *pass by reference*. The function stops and exits when the boolean variable is true. You need to manipulate this variable using interrupts. Note that the **rpm input can be signed**. If negative, the motor will rotate in the reverse direction.

* `void seek_position(float target_angle, int max_duty_cycle bool& exit_variable)` is the function to call for position control. The motor will try to maintain its position at the specified angle as long as the function is under execution. The second argument helps adjust the maximum speed at which the motor will move throughout the entire course of its action, on a scale from 0-255. The inputs can be signed and follows the usual conventions.  The third argument is a boolean variable that you need to *pass by reference*. The function stops and exits when the boolean variable is true. You need to manipulate this variable using interrupts.

## Example Use

This code will make the motor go to and fro from its home position to 5400 degrees. Each motion sequence is executed when an interrupt is triggered due to a falling edge at pin 2.

    #include "driver.h"
    #include "encoder.h"
    #include "motorcontrol.h"
    #include<Wire.h>

    motorController c1(A1, 9, 4, 6, A3);
    bool exit_var;

    void setup(){
    Serial.begin(115200);
    
    Wire.begin();
        c1.init();
        pinMode(2, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(2), routine, FALLING);
        exit_var = false;
    }

    void loop(){
        exit_var = false;
        c1.seek_position(5400, 100, exit_var);
        exit_var = false;
        c1.seek_position(0, 255, exit_var);
    }

    void routine(){
        exit_var = true;
        Serial.println("CALLED");
    }

## TO-DO

* Improve noise reduction for velocity
* Add functionality to end a control command after a fixed amount of time, which can be passed as an argument
* Implement time-varying target seeking
* Implementation on a multi-core microcontroller
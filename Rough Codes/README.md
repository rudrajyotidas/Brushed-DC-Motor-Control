# Testing Codes

The codes presented in this folder help us check the working of our implementation on hardware. Now that the results have been verified, we will move forward to writing a modular version whatever presented here

## Files and their purpose

### **AS5600_MagnetDistanceCheck** 

Upload this code to your Arduino, and check whether the distance between the encoder and the magnet is alright by looking at the Serial Monitor. There are three indicators, **'HIGH'**, **'LOW'** and **'DETECTED'**. In an ideal situation, **'DETECTED'** should be 1 and the other two should be 0. If **'HIGH'** is 1, increase the separation between the encoder and the magnet, and if **'LOW'** is 1, decrease it.

### **AS5600_ReadAngles**

Once the ideal separation is determined, upload this code to your Arduino, rotate the motor shaft and look at the Serial Monitor. You can see the angle readings there. If you observe any of the following, your magnet might be off centre, or your encoder might not be aligned with the magnet, or your encoder might not be parallel to the magnet:

* On rotating the entire setup while keeping the shaft stationary with respect to the motor, angle readings are changing drastically (more than 4-5 degrees)

* On rotating the shaft the angles being shown at the Serial Monitor are not as per your expectations

### **Current_Control_filtered**

Code for current control. The current sense readings from the Sparkfun Monster Motor Shield are converted to the actual current flowing through the coils according the datasheet. Some further callibrations can be done based on multimeter readings. The readings being very noisy, are filtered by a digital low pass filter. A simple PI control with some anti-windup (very much necessary) is sufficient for good responses.

Some things to note:

* Anti-windup is necessary, and if removed there will be a huge overshoot. But adding an anti-windup seem to result in oscillations in motor rpm (current readings still remain same). This can be eliminated by increasing the Ki, or modifying the windup code.

* The no-load current of the motor at max rpm is around 0.36A. So if your current reference is above 0.36A, and you're not applying any external torque, the current will always stay at 0.36A (this is why the anti-windup causes those rpm oscillations). If your current reference is 0.6A, that means upon application of sufficient external torque, the motor will stall while drawing 0.6A from the power supply. Since current and torque are proportionally related at steady state, current control helps us control torque.

Results can be seen in the Serial Plotter

### **Basic_Motor_Velocity_Control**

Code for velocity control (without using current control). Obtaining velocity readings from the AS5600 encoder requires **two low pass filters** with different cutoff frequencies. The first filter is applied on the angle readings from the encoder to reject the noise due to vibrations in the setup and changes in the orientations of the setup (our setup shows fluctuations of 1-2 degrees even after taking all the measures explained in 2.). While differentiating the angle readings to find out the velocity, the low frequency noise that was passed by the previous low pass filter gets amplified. So we need to apply another low pass filter, with an even lower cut off frequency to reject this noise.

Once filtered velocity readings are obtained, we apply PID control to reach the required rpm. *The Kd term is necessary in this case, and eliminates overshoot completely*. The step response resembles a first order system response.

On applying external torque, the motor tries to seek back the reference due to the Ki term. The seeking back action is a bit slow however, if we apply the disturbance torque after reaching the steady state (rather than applying it from the beginning). This can be tried to be fixed by some further tuning.

The windup code also needs to be tuned, if we want the motor to rotate at high rpm under high external torques.

### **Position_Control_Using_Velocity_Control**

Code for position control. Uses a PD controller to give an rpm target, which is fed to the velocity controller. As of now, the frequency of the position and velocity control loops are same (typically, it is position control updates 10 times slower than velocity control to reduce use of computing power). The responses can be seen in the Serial Plotter, and resembles a first order system with a very fast response time.

**The response is very much sensitive on the Kd term**. Even a change as small as 0.1 results in significant differences in the response.

*Excellent disturbance rejection, on applying a sudden backward torque the motor fights back and almost does not move from the reference*.

## Connections

### Motor Driver and Arduino(only needed if not using as a shield)
*A1* of Arduino UNO to *A1* of Motor Driver \
*A3* of Arduino UNO to *A3* of Motor Driver \
*6* of Arduino UNO to *6* of Motor Driver \
*4* of Arduino UNO to *4* of Motor Driver \
*9* of Arduino UNO to *9* of Motor Driver \
*5V* of Arduino UNO to *5V* of Motor Driver \
*GND* of Arduino UNO to *GND* of Motor Driver 

### AS5600 and Arduino
*A4* of Arduino UNO to *SDA* \
*A5* of Arduino UNO to *SCL* \
*5V* of Arduino UNO to *5V* \
*GND* of Arduino UNO to *GND* \
*GND* of Arduino UNO to *DIR* 

### Motor, Motor Driver and Power Supply
*12V* to + terminal\
*GND* to - terminal\
Motor to *A2* and *B2*

**If code doesnt work properly, swap the connections of A2 and B2, because the code is uni-directional as of now**

## Things to do

* Bi-directional position control
* Adding a variable of maximum velocity, above which the velocity will never increase while trying to seek the reference. This will help us reduce the response time without needing to change the PID coefficients
* Determine motor constant, to extend current control to torque control
* Adding a velocity observer to obtain velocity readings without differentiating position readings (need to determine motor inductance for that)

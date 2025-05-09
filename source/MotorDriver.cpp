#include "MotorDriver.h"

#define MOTORRAMP 15
#define MAXDUTY 255
#define TARGETPWM 49
#define AIN1 21
#define AIN2 20
#define PWMA 25
#define BIN1 7
#define BIN2 8
#define PWMB 16
#define STBY 12
#define DUTY 150

MotorDriver::MotorDriver()
{

    gpioSetMode(AIN1,PI_OUTPUT); gpioSetMode(AIN2,PI_OUTPUT);
    gpioSetMode(BIN1,PI_OUTPUT); gpioSetMode(BIN2,PI_OUTPUT);
    gpioSetMode(PWMA,PI_OUTPUT); gpioSetMode(PWMB,PI_OUTPUT);
    gpioSetMode(STBY,PI_OUTPUT);
    stop();

}


void MotorDriver::prime()
{
    gpioWrite(STBY, 1);
    _currentPWM = 60;                       // first tick starts here
    gpioPWM(PWMA, _currentPWM);
    gpioPWM(PWMB, _currentPWM);
    _targetPWM = DUTY;                     // slider‑chosen top speed
}


void MotorDriver::setLeft (bool fwd)
{ gpioWrite(AIN1,fwd); gpioWrite(AIN2,!fwd); }


void MotorDriver::setRight(bool fwd)
{ gpioWrite(BIN1,fwd); gpioWrite(BIN2,!fwd); }


void MotorDriver::setSpeed(int duty)
{
    _targetPWM = std::clamp(duty, 0, MAXDUTY);
    if (_targetPWM < TARGETPWM && _targetPWM > 0) _targetPWM = TARGETPWM; // keep min torque
}


void MotorDriver::tick()
{
    if (_currentPWM == _targetPWM) return;           // already there

    int step = (_currentPWM < _targetPWM) ? MOTORRAMP : -MOTORRAMP; // change the two numbers to adjust soft start, e.g., 5 : -5 ≈ 820 ms; 10 : -10 ≈ 400 ms
    _currentPWM += step;

    /* overshoot guard */
    if ((step > 0 && _currentPWM > _targetPWM) ||
        (step < 0 && _currentPWM < _targetPWM))
        _currentPWM = _targetPWM;

    gpioPWM(PWMA, _currentPWM);
    gpioPWM(PWMB, _currentPWM);
}


void MotorDriver::forward()  
{ 
    setLeft(true);  
    setRight(true);  
    prime(); 
}
void MotorDriver::backward() 
{
    setLeft(false); 
    setRight(false); 
    prime(); 
}
void MotorDriver::left()     
{ 
    setLeft(false); 
    setRight(true);  
    prime(); 
}
void MotorDriver::right()    
{ 
    setLeft(true);  
    setRight(false); 
    prime(); 
}


void MotorDriver::stop()
{
    setSpeed(0);              // just set target; tick() will ramp down
    gpioWrite(AIN1,0); gpioWrite(AIN2,0);
    gpioWrite(BIN1,0); gpioWrite(BIN2,0);
    gpioWrite(STBY,0);
}


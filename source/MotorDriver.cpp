#include "MotorDriver.h"

#define MOTORRAMP 15
#define MAXDUTY 255
#define TARGETPWM 49

MotorDriver::MotorDriver(int AIN1, int AIN2, int pwmA, int BIN1, int BIN2, int pwmB, int stby, int duty)
:_AIN1(AIN1), _AIN2(AIN2), _pwmA(pwmA), _BIN1(BIN1), _BIN2(BIN2), _pwmB(pwmB), _stby(stby), _duty(duty)
{

    gpioSetMode(_AIN1,PI_OUTPUT); gpioSetMode(_AIN2,PI_OUTPUT);
    gpioSetMode(_BIN1,PI_OUTPUT); gpioSetMode(_BIN2,PI_OUTPUT);
    gpioSetMode(_pwmA,PI_OUTPUT); gpioSetMode(_pwmB,PI_OUTPUT);
    gpioSetMode(_stby,PI_OUTPUT);
    stop();
}


void MotorDriver::prime()
{
    gpioWrite(_stby, 1);
    _currentPWM = 60;                       // first tick starts here
    gpioPWM(_pwmA, _currentPWM);
    gpioPWM(_pwmB, _currentPWM);
    _targetPWM = _duty;                     // slider‑chosen top speed
}


void MotorDriver::setLeft (bool fwd)
{ gpioWrite(_AIN1,fwd); gpioWrite(_AIN2,!fwd); }


void MotorDriver::setRight(bool fwd)
{ gpioWrite(_BIN1,fwd); gpioWrite(_BIN2,!fwd); }


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

    gpioPWM(_pwmA, _currentPWM);
    gpioPWM(_pwmB, _currentPWM);
}


void MotorDriver::forward()  { setLeft(true);  setRight(true);  prime(); }
void MotorDriver::backward() { setLeft(false); setRight(false); prime(); }
void MotorDriver::left()     { setLeft(false); setRight(true);  prime(); }
void MotorDriver::right()    { setLeft(true);  setRight(false); prime(); }


void MotorDriver::stop()
{
    setSpeed(0);              // just set target; tick() will ramp down
    gpioWrite(_AIN1,0); gpioWrite(_AIN2,0);
    gpioWrite(_BIN1,0); gpioWrite(_BIN2,0);
    gpioWrite(_stby,0);
}


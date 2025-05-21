#pragma once

#include <pigpio.h>
#include <algorithm>

class MotorDriver
{
public:
    /* GPIO map defaults for:
       AIN1=4  AIN2=3  pwmA=2  (left motor)
       BIN1=27 BIN2=22 pwmB=10 (right motor)
       STBY=12
    */
    MotorDriver(int AIN1=4,  int AIN2=3,  int pwmA=2,
                int BIN1=27, int BIN2=22, int pwmB=10,
                int stby=17, int duty=150); // 0-255

    void forward();
    void backward();
    void left();
    void right();
    void stop();
    void setSpeed(int duty);
    void tick();               // call every frame for soft‑start/stop
    void prime();

private:
    int _AIN1, _AIN2, _pwmA, _BIN1, _BIN2, _pwmB, _stby, _duty;
    void setLeft(bool fwd);
    void setRight(bool fwd);

    int _currentPWM = 0;          // PWM actually applied
    int _targetPWM  = 0;          // desired PWM
};

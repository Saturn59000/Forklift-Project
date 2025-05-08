#pragma once

#include <pigpio.h>
#include <algorithm>

class MotorDriver
{
public:
    /* GPIO map defaults for:
       AIN1=21  AIN2=20  pwmA=16 (left motor)
       BIN1=8   BIN2=7   pwmB=24 (right motor)
       STBY=12
    */
    MotorDriver(int AIN1=21,  int AIN2=20,  int pwmA=16,
                int BIN1=7,   int BIN2=8,   int pwmB=24,
                int stby=12, int duty=150); // 0-255

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

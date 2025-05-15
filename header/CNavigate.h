#pragma once

#include "MotorDriver.h"
#include <opencv2/opencv.hpp>

class CNavigate
{
    private:

    double _tick_frequency;
    double _start_tick;
    bool _start_flag;
    MotorDriver _motor;

    public:
    CNavigate();
    ~CNavigate();

    void forward(double time);
    void backward(double time);
    void right(double time);
    void left(double time);
};


#include "CNavigate.h"

CNavigate::CNavigate()
{
_tick_frequency = cv::getTickFrequency();
_start_flag = false; 
}

CNavigate::~CNavigate()
{

}

void CNavigate::forward(double time)
{
double current_tick;

if (!_start_flag)
{
    _start_tick = cv::getTickCount();
    _start_flag = true;
}

_motor.forward();

current_tick = cv::getTickCount();

if ((current_tick - _start_tick) / _tick_frequency > time)
{
    _motor.stop();
    _start_flag = false; 
}
else 
    std::cout << "moving forward";
}

void CNavigate::backward(double time)
{
    
}

void CNavigate::right(double time)
{
    
}

void CNavigate::left(double time)
{
    
}

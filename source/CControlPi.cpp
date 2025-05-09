#include "CControlPi.h"


CControlPi::CControlPi()
{
    _program_init = true;
    _comm_failed = false;
    gpioInitialise();

    _last_button1_state = 1;
    _last_button2_state = 1;
    _start_time1 = 0;
    _start_time2 = 0;
}

CControlPi::~CControlPi()
{

    set_data(DIGITAL, BLUE_LED_CH, 0);
    set_data(DIGITAL, GREEN_LED_CH, 0);
    set_data(DIGITAL, RED_LED_CH, 0);

    gpioTerminate();

}

bool CControlPi::get_data(int type, int channel, int& result)
{
    if (gpioInitialise() < 1)
        return 1;

    gpioSetMode(channel, PI_INPUT);

    if (type == DIGITAL)
    {
        result = gpioRead(channel);
        return true;
    }
    else if (type == ANALOG)
    {
        int read_val;
        unsigned char inBuf[3];
        char cmd[] = {1, 8 + channel << 4, 0}; //0b1XXX0000 where XXX = channel 0

        int handle = spiOpen(0, 200000, 3);

        spiXfer(handle, cmd, (char*) inBuf, 3);
        read_val = ((inBuf[1] & 3) << 8) | inBuf[2];

        spiClose(handle);

        if (read_val - ADC_OFFSET< 0)
            result = 0;
        else
            result = read_val - ADC_OFFSET; //STE

        return true;
    }
    else if (type == SERVO)
    {
        //return last position
        return true;
    }
    else
        return false;
}

bool CControlPi::set_data(int type, int channel, int val)
{
    if (gpioInitialise() < 0)
        return 1;

    gpioSetMode(channel, PI_OUTPUT);
    if (type == DIGITAL)
    {
        gpioWrite(channel, val);
        return true;
    }
    else if (type == ANALOG)
    {
        //no set analog
        return true;
    }
    else if (type == SERVO)
    {
        int convert_us = val *(2000/180) + 500;
        gpioServo(channel, convert_us);
        return true;
    }
    else
        return false;
}

float CControlPi::get_analog(int channel)
{
    int result;

    get_data(ANALOG, channel, result);

    float result_percent = (static_cast <float>(result) / (ADC_MAX)) * PERCENT;


    return result_percent;


    return 0;
}

bool CControlPi::get_button(int channel)
{
    int button_value;
    float current_time;

    get_data(DIGITAL, channel, button_value);

    if ((button_value != _last_button1_state) && _start_time1 == 0)
        _start_time1 = cv::getTickCount() / cv::getTickFrequency();

    _last_button1_state = button_value;

    current_time = cv::getTickCount() / cv::getTickFrequency();

    if ((current_time - _start_time1 >= DEBOUNCE) && button_value == 1)
    {
        _start_time1 = 0;
        return true;
    }

    return false;
}

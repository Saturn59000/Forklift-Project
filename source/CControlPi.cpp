#include "CControlPi.h"


CControlPi::CControlPi() {}


CControlPi::~CControlPi() {}


bool CControlPi::get_data(int channel, int& result)
{
    gpioSetMode(channel, PI_INPUT);
    result = gpioRead(channel);
    return 1;
}


bool CControlPi::set_data(int channel, int val)
{
    gpioSetMode(channel, PI_OUTPUT);
    gpioWrite(channel, val);
    return 1;
}


double CControlPi::get_analog(int channel)
{
    int read_val;
    unsigned char inBuf[3];
    char cmd[] = { 1, channel, 0 };

    int handle = spiOpen(0, 200000, 3); // Open SPI channel 0 with 200kHz speed

    spiXfer(handle, cmd, (char*) inBuf, 3); // Transfer 3 bytes
    read_val = ((inBuf[1] & 3) << 8) | inBuf[2]; // Format 10 bits

    spiClose(handle);

    return read_val;
}


int CControlPi::get_button(int channel)
{
    gpioSetMode(channel, PI_INPUT);

    static int Intial_State = 1;
    int Current_State;
    int Not_Pushed = 1; //pull-up resister
    int Pushed = 0;

    static double Start_Tic = cv::getTickCount();

    Current_State = gpioRead(channel);

    if ((Intial_State == Not_Pushed) && (Current_State == Pushed) && ((cv::getTickCount() - Start_Tic) / cv::getTickFrequency() > 0.2)) //wait 200ms
      {
      Start_Tic = cv::getTickCount();
      Intial_State = Pushed;
      return Pushed;
      }
   else
      {
      Intial_State = Current_State;
      return Not_Pushed;
      }
}


int CControlPi::get_servo(int channel, int& position)
{
    position = gpioGetServoPulsewidth(channel); // Read servo pulse width

    return 1; // Success
}


bool CControlPi::set_servo(int channel, int val)
{
    gpioSetMode(channel, PI_OUTPUT);
    gpioServo(channel, val);
    return 1;
}

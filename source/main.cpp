#include "Forklift.h"
#include <pigpio.h>
#include <iostream>

int main()
{
    if (gpioInitialise() < 0)
    {
        std::cerr << "ERROR: pigpio init failed.\n";
        return -1;
    }

    {
        CForklift bot;
        bot.run();                      // inherited loop: update/draw
    }

    gpioTerminate();
    return 0;
}

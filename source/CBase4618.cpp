#include "CBase4618.h"

void CBase4618::run()
{
    while (true)
    {
        update();                   // capture & network
        draw();                     // GUI

        if (_exit)         // Exit button pressed?
        {
            break;                  // leave loop → call destructor
        }

        char c = (char)cv::waitKey(20);
        if (c == 'q' || c == 27)    // ‘q’ or Esc still work too
        {
            break;
        }
    }
}

#pragma once

#include "server.h"
#include "cvui.h"
#include "CCamera.h"
#include "CAruco.h"
#include <pigpio.h>
#include <deque>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <algorithm>
#include <cstring>
#include "CBase4618.h"
#include "CControlPi.h"
#include "MotorDriver.h"
#include "CNavigate.h"


#include "UdpFeedSender.h"

/* Forklift.h  (or Forklift.cpp where the driver is constructed)
 *             replace the old constructor call               */

#define PORT_FEED = 4618;
#define PORT_CMD  = 4620;

/* ───────────────── Forklift application ───────────────── */
class CForklift : public CBase4618
{
public:
    CForklift();
    ~CForklift();

    void update() override;
    void draw()   override;

    double _speed = 255;          // current GUI value

private:
    /* networking */
    UdpFeedSender _udp;
    CServer _srvCmd;
    CAruco _aruco;
    CNavigate _nav;

    bool _run_once; 

    cv::Size _FEED_SIZE = cv::Size(320,420);
    // Servo
    unsigned _servoGpio = 18;          // GPIO15 → PWM0 pin 12 (you asked for 18)
    unsigned _pulseUp   = 1000;        // µs for “Up”
    unsigned _pulseDown = 1300;        // µs for “Down”

    /* vision */
    //CCamera _cam;
    cv::VideoCapture _cap;
    cv::Mat _frame;

    /* control */
    MotorDriver      _drive;
    bool             _autoMode   = false;
    bool             _clientSeen = false;
    std::chrono::steady_clock::time_point _lastCmdT;

    /* gui */
    cv::Mat                 _canvas;
    std::deque<std::string> _log;      // last 8 cmds
    std::mutex              _logMtx;

    /* helpers */
    void handleCommands();
    void pushLog(std::string s);
};

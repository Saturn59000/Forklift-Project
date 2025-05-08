#pragma once

#include "server.h"
#include "cvui.h"
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

/* Forklift.h  (or Forklift.cpp where the driver is constructed)
 *             replace the old constructor call               */

constexpr int PORT_FEED = 4618;
constexpr int PORT_CMD  = 4620;


/* ───────────────── Forklift application ───────────────── */
class CForklift : public CBase4618
{
public:
    CForklift();
    ~CForklift();

    void update() override;
    void draw()   override;

    double _speed = 150;          // current GUI value

private:
    /* networking */
    CServer _srvFeed;
    CServer _srvCmd;

    // Servo
    unsigned _servoGpio = 18;          // GPIO15 → PWM0 pin 12 (you asked for 18)
    unsigned _pulseUp   = 1000;        // µs for “Up”
    unsigned _pulseDown = 1300;        // µs for “Down”

    /* vision */
    cv::VideoCapture _cap;
    cv::Mat          _frame;

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

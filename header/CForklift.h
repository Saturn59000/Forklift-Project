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
#include "CCamera.h"
#include "CAruco.h"
#include "CNavigate.h"
#include "UdpFeedSender.h"

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
    UdpFeedSender _udp;
    CServer _srvCmd;
    CAruco _aruco;
    CNavigate _nav;
    CControlPi _control;

    bool _run_once;

    // Servo
    unsigned _servoGpio = 9;        // GPIO9 to J1 on Pi hat
    unsigned SERVO_MIN_US =  700;   // 0 °  absolute minimum
    unsigned SERVO_MIN_STEP = 800;
    unsigned SERVO_MAX_US = 1500;   // 180° absolute maximum
    unsigned SERVO_STEP_US = 100;   // equal 60° steps

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

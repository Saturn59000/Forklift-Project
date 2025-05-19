#include "stdafx.h"          // keep if your project uses PCH
#define CVUI_DISABLE_COMPILATION_NOTICES
#define CVUI_IMPLEMENTATION
#include "cvui.h"

#include "ForkliftGUI.h"
#include <windows.h>
#include <sstream>

static constexpr const char* WINDOW_NAME = "Forklift Client";
using Clock = std::chrono::steady_clock;

/* ─── ctor / dtor ─── */
ForkliftGUI::ForkliftGUI(const char* ip, int pc, int pf)
    : _piIP(ip), _PORT_CMD(pc), _PORT_FEED(pf),
    _ui(cv::Size(640, 500), CV_8UC3)
{
    cvui::init(WINDOW_NAME);
}

ForkliftGUI::~ForkliftGUI()
{
    _stopFeed = true; if (_feedT.joinable()) _feedT.join();
    _cmdCli.close_socket();
}

/* ─── static key helper ─── */
bool ForkliftGUI::isKeyDown(int vk)
{
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

/* ─── thin send wrapper ─── */
void ForkliftGUI::send(const std::string & s)
{
   if (!_connected) return;
   
   /* non-blocking send; drop if the buffer is momentarily full */
   const char* p = s.c_str();
   size_t      n = s.size();
   int sent = ::send(_cmdCli.sock(), p, (int)n, 0);
   (void)sent;            // we don’t retry – next frame sends again if needed
 }

/* ─── feed thread ─── */
void ForkliftGUI::feedThread()
{
    UdpFeedReceiver recv(4619, _piIP, _PORT_FEED);
    while (!_stopFeed)
    {
        cv::Mat img;
        if (recv.grab(img))
        {
            std::lock_guard<std::mutex> lk(_imMtx); _shared = img.clone();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

/* ─── GUI draw helpers (unchanged logic) ─── */
void ForkliftGUI::drawDriveButtons(int y0, int bw, int bh)
{
    auto hi = [&](int x, int y, const std::string& tag)
        {
            bool show = false;
            auto now = Clock::now();
            if (_flash.count(tag) && now - _flash[tag] < std::chrono::milliseconds(100))
                show = true;
            if (tag == "UP" && isKeyDown(VK_UP))      show = true;
            if (tag == "DOWN" && isKeyDown(VK_DOWN))    show = true;
            if (tag == "LEFT" && isKeyDown(VK_LEFT))    show = true;
            if (tag == "RIGHT" && isKeyDown(VK_RIGHT))   show = true;
            if (tag == "STOP" && isKeyDown(VK_SPACE))   show = true;
            if (show)
                cv::rectangle(_ui, cv::Rect(x - 1, y - 1, bw + 2, bh + 2),
                    (tag == "STOP") ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0), 2);
        };
    if (cvui::button(_ui, 110, y0, bw, bh, "Up")) { send("UP\n");   _flash["UP"] = Clock::now(); }
    if (cvui::button(_ui, 40, y0 + 40, bw, bh, "Left")) { send("LEFT\n"); _flash["LEFT"] = Clock::now(); }
    if (cvui::button(_ui, 111, y0 + 40, bw, bh, "Down")) { send("DOWN\n"); _flash["DOWN"] = Clock::now(); }
    if (cvui::button(_ui, 182, y0 + 40, bw, bh, "Right")) { send("RIGHT\n"); _flash["RIGHT"] = Clock::now(); }
    if (cvui::button(_ui, 110, y0 + 80, bw, bh, "Stop")) { send("STOP\n"); _flash["STOP"] = Clock::now(); }
    hi(110, y0, "UP"); hi(40, y0 + 40, "LEFT"); hi(111, y0 + 40, "DOWN"); hi(182, y0 + 40, "RIGHT"); hi(110, y0 + 80, "STOP");
}

void ForkliftGUI::drawForkButtons(int y0, int bw, int bh)
{
    int bwBig = bw + 30;
    auto forkBtn = [&](int pos, int x, const std::string& lbl)
        {
            std::string tag = "F" + std::to_string(pos);
            if (cvui::button(_ui, x, y0, bwBig, bh, lbl) || isKeyDown('0' + pos))
            {
                send("FORK" + std::to_string(pos) + "\n"); _flash[tag] = Clock::now();
            }
            auto now = Clock::now();
            if (_flash.count(tag) && now - _flash[tag] < std::chrono::milliseconds(100))
                cv::rectangle(_ui, cv::Rect(x - 1, y0 - 1, bwBig + 2, bh + 2),
                    cv::Scalar(0, 255, 255), 2);
        };
    forkBtn(1, 20, "Fork Pos 1");
    forkBtn(2, 121, "Fork Pos 2");
    forkBtn(3, 222, "Fork Pos 3");
    forkBtn(4, 323, "Fork Pos 4");
}

void ForkliftGUI::drawModeToggles(int y0)
{
    int activeX = _modeAuto ? 20 : 160;
    cv::rectangle(_ui, cv::Rect(activeX - 2, y0 - 2, 124, 34), { 0,100,0 }, cv::FILLED);
    if (cvui::button(_ui, 20, y0, 120, 30, "Auto Mode")) { send("MODE AUTO\n");   _modeAuto = true; }
    if (cvui::button(_ui, 160, y0, 120, 30, "Manual Mode")) { send("MODE MANUAL\n"); _modeAuto = false; }
}

void ForkliftGUI::drawFeedButtons(int y0)
{
    if (cvui::button(_ui, 20, y0, 120, 30, "Start Feed") && !_feedRunning && _connected)
    {
        _stopFeed = false; _feedT = std::thread(&ForkliftGUI::feedThread, this); _feedRunning = true;
    }
    if (cvui::button(_ui, 160, y0, 120, 30, "Stop Feed") && _feedRunning)
    {
        _stopFeed = true; _feedT.join(); _feedRunning = false;
    }
}

void ForkliftGUI::drawVideo()
{
    std::lock_guard<std::mutex> lk(_imMtx);
    if (_shared.empty()) return;
    cv::Mat roi = _ui(cv::Rect(380, 50, 240, 180));
    cv::resize(_shared, roi, roi.size());
    cv::rectangle(_ui, cv::Rect(380, 50, 240, 180), { 0,255,0 }, 2);
    cvui::text(_ui, 390, 60, "Remote feed");
}

/* ─── main loop ─── */
void ForkliftGUI::run()
{
    while (!_quit)
    {
        _ui = cv::Scalar(45, 45, 45);
        cvui::text(_ui, 20, 20, "Forklift Client", 0.6, 0x00ff00);

        /* connect / disconnect */
        if (cvui::button(_ui, 20, 60, 120, 30, "Connect") && !_connected)
        {
            _cmdCli.connect_socket(_piIP, _PORT_CMD); _connected = true;
        }
        if (cvui::button(_ui, 160, 60, 120, 30, "Disconnect") && _connected)
        {
            if (_feedRunning) { _stopFeed = true; _feedT.join(); _feedRunning = false; }
            _cmdCli.close_socket(); _connected = false;
        }

        /* controls */
        int y0 = 120, bw = 70, bh = 30;
        if (!_modeAuto) { drawDriveButtons(y0, bw, bh); drawForkButtons(y0 + 140, bw, bh); }

        /* speed slider */
        cvui::trackbar(_ui, 380, 350, 200, &_speed, 50, 255);
        if (_connected && !_modeAuto && _speed != _lastSent)
        {
            std::ostringstream os; os << "SPD" << _speed << "\n"; send(os.str()); _lastSent = _speed;
        }

        drawModeToggles(305);
        drawFeedButtons(350);
        drawVideo();

        if (cvui::button(_ui, 20, 420, 260, 30, "Exit")) _quit = true;

        /* --- EXTRA: press-and-hold arrow keys + space bar (Windows only) --- */
#ifdef _WIN32
        static int  lastDir = 0;  // 0=none 1-Up 2-Down 3-Left 4-Right
        static bool stopSent = true;

        auto pressed = [&](int vk) { return GetAsyncKeyState(vk) & 0x8000; };

        int dir = 0;
        if (!_modeAuto && _connected)
        {
            if (pressed(VK_UP))    dir = 1;
            else if (pressed(VK_DOWN))  dir = 2;
            else if (pressed(VK_LEFT))  dir = 3;
            else if (pressed(VK_RIGHT)) dir = 4;
        }

        if (dir != lastDir)
        {
            switch (dir)
            {
            case 1: send("UP\n");    _flash["UP"]    = Clock::now(); break;
            case 2: send("DOWN\n");  _flash["DOWN"]  = Clock::now(); break;
            case 3: send("LEFT\n");  _flash["LEFT"]  = Clock::now(); break;
            case 4: send("RIGHT\n"); _flash["RIGHT"] = Clock::now(); break;
            default:
                if (!stopSent) { send("STOP\n"); _flash["STOP"] = Clock::now(); stopSent = true; }
            }
            lastDir = dir; stopSent = (dir == 0);
        }

        if (!_modeAuto && _connected && pressed(VK_SPACE) && !stopSent)
        {
            send("STOP\n"); _flash["STOP"] = Clock::now(); stopSent = true;
        }
#endif

        cvui::update();
        cv::imshow(WINDOW_NAME, _ui);
        if (cv::waitKey(1) == 27) _quit = true;
    }
}

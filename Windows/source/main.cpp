/***********************************************************************
 * Forklift.cpp  -  Windows client with keyboard + GUI control
 **********************************************************************/
#include "stdafx.h"

#define CVUI_DISABLE_COMPILATION_NOTICES
#define CVUI_IMPLEMENTATION
#include "cvui.h"

#include "Client.h"
#include "UdpFeedReceiver.h"
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <windows.h>
#include <sstream>


 /* ------------ Settings ------------ */
static const char* PI_IP = "10.0.0.91";
static const int   PORT_CMD = 4620;
static const int   PORT_FEED = 4618;
static const int   IMG_W = 320, IMG_H = 240;
#define WINDOW_NAME "Forklift Client"


/* ------------ Small helpers ------------ */
using Clock = std::chrono::steady_clock; // helper for highlight expiration
bool isKeyDown(int vk)
{
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}


/* -------------------- Feed thread -------------------- */
void feedThread(std::atomic<bool>& stop, cv::Mat& shared, std::mutex& mtx)
{
    UdpFeedReceiver recv(4619, PI_IP, PORT_FEED);   // pick any free local port (4619)

    while (!stop)
    {
        cv::Mat img;
        if (recv.grab(img))
        {
            std::lock_guard<std::mutex> lk(mtx);
            shared = img.clone();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}


/* ------------------------------------------------------------------ */
int main()
{
    cv::Mat ui(cv::Size(640, 500), CV_8UC3);
    cvui::init(WINDOW_NAME);

    CClient cmdCli;
    bool connected = false, feedRunning = false;

    std::thread feedT;
    std::atomic<bool> stopFeed{ false };
    std::mutex imMtx; cv::Mat shared;

    bool modeAuto = false, quit = false;

    int speed = 150;           // shared with slider
    int lastSent = speed;

    /* flags for GUI highlight */
    std::unordered_map<std::string, Clock::time_point> flash;

    while (!quit)
    {
        ui = cv::Scalar(45, 45, 45);
        cvui::text(ui, 20, 20, "Forklift Client", 0.6, 0x00ff00);

        /* --------------------- Connect block --------------------- */
        if (cvui::button(ui, 20, 60, 120, 30, "Connect") && !connected)
        {
            cmdCli.connect_socket(PI_IP, PORT_CMD);
            connected = true;
        }
        if (cvui::button(ui, 160, 60, 120, 30, "Disconnect") && connected)
        {
            if (feedRunning) { stopFeed = true; feedT.join(); feedRunning = false; }
            cmdCli.close_socket();
            connected = false;
        }

        /* ------------------------ Drive + STOP buttons ------------------------ */
        int y0 = 120, bw = 70, bh = 30; auto now = Clock::now();
        auto highlight = [&](int x, int y, int w, int h, const std::string& tag)
            {
                bool show = false;

                // show if recent click (flash) OR if key is held
                if (flash.count(tag) && Clock::now() - flash[tag] < std::chrono::milliseconds(100))
                    show = true;

                // persistent highlight while key is held
                if (tag == "UP" && isKeyDown(VK_UP))    show = true;
                if (tag == "DOWN" && isKeyDown(VK_DOWN))  show = true;
                if (tag == "LEFT" && isKeyDown(VK_LEFT))  show = true;
                if (tag == "RIGHT" && isKeyDown(VK_RIGHT)) show = true;
                if (tag == "STOP" && isKeyDown(VK_SPACE)) show = true;
                if (tag == "FUP" && isKeyDown('W'))      show = true;
                if (tag == "FDN" && isKeyDown('S'))      show = true;

                if (show)
                {
                    cv::Scalar col =
                        (tag == "STOP") ? cv::Scalar(0, 0, 255) :   // red
                        (tag == "FUP" || tag == "FDN") ? cv::Scalar(0, 255, 255) : // yellow
                        cv::Scalar(0, 255, 0);  // green

                    cv::rectangle(ui, cv::Rect(x - 1.01, y - 1, w + 2, h + 2), col, 2);
                }
            };

        if (!modeAuto)
        {
            if (cvui::button(ui, 110, y0, bw, bh, "Up") && connected)
            {
                cmdCli.tx_str("UP\n");   flash["UP"] = now;
            }
            highlight(110, y0, bw, bh, "UP");

            if (cvui::button(ui, 40, y0 + 40, bw, bh, "Left") && connected)
            {
                cmdCli.tx_str("LEFT\n"); flash["LEFT"] = now;
            }
            highlight(40, y0 + 40, bw, bh, "LEFT");

            if (cvui::button(ui, 111, y0 + 40, bw, bh, "Down") && connected)
            {
                cmdCli.tx_str("DOWN\n"); flash["DOWN"] = now;
            }
            highlight(111, y0 + 40, bw, bh, "DOWN");

            if (cvui::button(ui, 182, y0 + 40, bw, bh, "Right") && connected)
            {
                cmdCli.tx_str("RIGHT\n"); flash["RIGHT"] = now;
            }
            highlight(182, y0 + 40, bw, bh, "RIGHT");

            if (cvui::button(ui, 110, y0 + 80, bw, bh, "Stop") && connected)
            {
                cmdCli.tx_str("STOP\n"); flash["STOP"] = now;
            }
            highlight(110, y0 + 80, bw, bh, "STOP");

            /* ------------------ Fork-lift servo buttons ------------------ */
            int yServo = y0 + 140;           // place one row below Stop
            if (cvui::button(ui, 20, yServo, bw + 50, bh, "Fork Up") && connected)
            {
                cmdCli.tx_str("FORKUP\n");
                flash["FUP"] = now;
            }

            int wFork = bw + 50;
            highlight(20, yServo, wFork, bh, "FUP");

            if (cvui::button(ui, 160, yServo, bw + 50, bh, "Fork Down") && connected)
            {
                cmdCli.tx_str("FORKDOWN\n");
                flash["FDN"] = now;
            }
            highlight(160, yServo, wFork, bh, "FDN");

            /* ------------------ Speed slider ------------------ */
            cvui::trackbar(ui, 380, 350, 200, &speed, 50, 255);
            cvui::text(ui, 455, 325, "Speed");

            /* send only when value changes by at least 1 */
            if (connected && !modeAuto && int(speed) != int(lastSent))
            {
                std::ostringstream os;
                os << "SPD" << int(speed) << "\n";
                cmdCli.tx_str(os.str());
                lastSent = speed;
            }
        }
        else
        {
            cvui::text(ui, 30, y0 + 30, "Buttons disabled in Auto");
        }

        /* --- Mode buttons --- */
        y0 = 305; const int BTN_W = 120, BTN_H = 30;
        int activeX = modeAuto ? 20 : 160;
        cv::rectangle(ui, cv::Rect(activeX - 2, y0 - 2, BTN_W + 4, BTN_H + 4), { 0,100,0 }, cv::FILLED);
        if (cvui::button(ui, 20, y0, BTN_W, BTN_H, "Auto Mode"))   modeAuto = true;
        if (cvui::button(ui, 160, y0, BTN_W, BTN_H, "Manual Mode"))modeAuto = false;

        static bool lastMode = modeAuto;
        if (connected && modeAuto != lastMode)
        {
            cmdCli.tx_str(modeAuto ? "MODE AUTO\n" : "MODE MANUAL\n");
            lastMode = modeAuto;
        }

        /* ------------------------ Feed controls ------------------------ */
        y0 = 350;
        if (cvui::button(ui, 20, y0, 120, 30, "Start Feed") && connected && !feedRunning)
        {
            stopFeed = false;
            feedT = std::thread(feedThread, std::ref(stopFeed), std::ref(shared), std::ref(imMtx));
            feedRunning = true;
        }
        if (cvui::button(ui, 160, y0, 120, 30, "Stop Feed") && feedRunning)
        {
            stopFeed = true; feedT.join(); feedRunning = false;
        }

        /* --------------- Show frame --------------- */
        {
            std::lock_guard<std::mutex> lk(imMtx);
            if (!shared.empty())
            {
                cv::Mat roi = ui(cv::Rect(380, 50, 240, 180));
                cv::resize(shared, roi, roi.size());
                cv::rectangle(ui, cv::Rect(380, 50, 240, 180), { 0,255,0 }, 2);
                cvui::text(ui, 390, 60, "Remote feed");
            }
        }

        /* ------------------------ Exit ------------------------ */
        if (cvui::button(ui, 20, 420, 260, 30, "Exit")) quit = true;

        /* --- EXTRA: poll arrow keys for Windows (Manual mode only) --- */
        #ifdef _WIN32
                /* -------- press‑and‑hold keys --------
                   Arrow   = drive           (continuous)
                   Space   = STOP            (continuous)
                   W or w  = Fork Up         (edge‑trigger)
                   S or s  = Fork Down       (edge‑trigger)          */
                static int  lastDir = 0;    // 0 none 1‑Up 2‑Down 3‑Left 4‑Right
                static bool stopSent = true;

                auto pressed = [](int vk) { return GetAsyncKeyState(vk) & 0x8000; };

                int dir = 0;
                if (!modeAuto && connected)
                {
                    if (pressed(VK_UP))    dir = 1;
                    else if (pressed(VK_DOWN))  dir = 2;
                    else if (pressed(VK_LEFT))  dir = 3;
                    else if (pressed(VK_RIGHT)) dir = 4;
                }

                /* direction keys (hold behaviour) */
                if (dir != lastDir)
                {
                    switch (dir)
                    {
                    case 1: cmdCli.tx_str("UP\n");    flash["UP"] = Clock::now(); break;
                    case 2: cmdCli.tx_str("DOWN\n");  flash["DOWN"] = Clock::now(); break;
                    case 3: cmdCli.tx_str("LEFT\n");  flash["LEFT"] = Clock::now(); break;
                    case 4: cmdCli.tx_str("RIGHT\n"); flash["RIGHT"] = Clock::now(); break;
                    default:
                        if (!stopSent)
                        {
                            cmdCli.tx_str("STOP\n");  flash["STOP"] = Clock::now();
                            stopSent = true;
                        }
                    }
                    lastDir = dir;
                    stopSent = (dir == 0);
                }

                /* space bar = STOP (continuous) */
                if (!modeAuto && connected && pressed(VK_SPACE) && !stopSent)
                {
                    cmdCli.tx_str("STOP\n");  flash["STOP"] = Clock::now();
                    stopSent = true;
                }

                /* Fork controls: edge‑trigger (send once per press) */
                static SHORT lastW = 0, lastS = 0;
                SHORT nowW = GetAsyncKeyState('W');
                SHORT nowS = GetAsyncKeyState('S');

                auto edge = [](SHORT now, SHORT last) { return (now & 0x8000) && !(last & 0x8000); };

                if (!modeAuto && connected)
                {
                    if (edge(nowW, lastW)) { cmdCli.tx_str("FORKUP\n");   flash["FUP"] = Clock::now(); }
                    if (edge(nowS, lastS)) { cmdCli.tx_str("FORKDOWN\n"); flash["FDN"] = Clock::now(); }
                }
                lastW = nowW; lastS = nowS;
        #endif


        /* --------- Render --------- */
        cvui::update();
        cv::imshow(WINDOW_NAME, ui);

        int k = cv::waitKey(1);
        if (k == 27) quit = true;  // Esc quits
    }

    if (feedRunning) { stopFeed = true; feedT.join(); }
    cmdCli.close_socket();
    return 0;
}
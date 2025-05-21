#pragma once
/***********************************************************************
 * ForkliftGUI.h — Windows-side forklift controller
 **********************************************************************/
#include "Client.h"
#include "UdpFeedReceiver.h"
#include <opencv2/opencv.hpp>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_map>

class ForkliftGUI
{
public:
    ForkliftGUI(const char* pi_ip = "192.168.137.32",
        int port_cmd = 4620,
        int port_feed = 4618);
    ~ForkliftGUI();

    void run();                       // main loop

private:
    /* ---------- networking ---------- */
    const char* _piIP;
    const int   _PORT_CMD, _PORT_FEED;
    CClient     _cmdCli;
    bool        _connected = false;

    /* ---------- feed thread ---------- */
    void feedThread();
    std::thread               _feedT;
    std::atomic<bool>         _stopFeed{ false };
    std::mutex                _imMtx;
    cv::Mat                   _shared;
    bool                      _feedRunning = false;

    /* ---------- GUI state ---------- */
    cv::Mat                   _ui;
    int                       _speed = 150;
    int                       _lastSent = 150;
    bool                      _modeAuto = false;
    bool                      _quit = false;
    std::unordered_map<std::string,
        std::chrono::steady_clock::time_point> _flash;

    /* ---------- helpers ---------- */
    static bool isKeyDown(int vk);
    void send(const std::string& s);

    void drawDriveButtons(int y0, int bw, int bh);
    void drawForkButtons(int y0, int bw, int bh);
    void drawModeToggles(int y0);
    void drawFeedButtons(int y0);
    void drawVideo();
};

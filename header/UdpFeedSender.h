#pragma once
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


class UdpFeedSender
{
public:
    void start (int port);                 // spawns worker thread
    void stop  ();
    void setFrame (const cv::Mat& im);     // called by your main loop

private:
    void loop (int port);

    int                 _sock = -1;
    sockaddr_in         _clientAddr{};     // set after first HELLO
    std::atomic<bool>   _haveClient{false};
    std::atomic<bool>   _exit{false};

    std::thread         _th;
    std::mutex          _mx;
    cv::Mat             _frame;            // latest frame
};

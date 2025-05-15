#pragma once
#include <opencv2/opencv.hpp>
#include <WinSock2.h>
#include <chrono>
#include <thread>

class UdpFeedReceiver
{
public:
    UdpFeedReceiver(int localPort, const char* piAddr, int piPort);
    ~UdpFeedReceiver();
    bool grab(cv::Mat& out);        // returns true if got new frame

private:
    SOCKET      _sock = INVALID_SOCKET;
    sockaddr_in _pi{};
    std::vector<uchar> _buf;
};

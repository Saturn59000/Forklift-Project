#pragma once

#include <opencv2/opencv.hpp>
#include <winsock2.h>

class UdpFeedReceiver {
public:
    UdpFeedReceiver(uint16_t port);
    ~UdpFeedReceiver();
    bool grab(cv::Mat& out);            // non‑blocking, true → new image
private:
    SOCKET     _s;
    uint32_t   _lastSeq = 0;
    char       _buf[65535];
};

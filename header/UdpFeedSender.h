#pragma once

#include <opencv2/opencv.hpp>
#include <arpa/inet.h>

class UdpFeedSender {
public:
    UdpFeedSender(const char* clientIP, uint16_t port);
    ~UdpFeedSender();
    void send(const cv::Mat& frame);          // call once per captured frame
private:
    int          _sock;
    sockaddr_in  _cli{};
    uint32_t     _seq = 0;
    std::vector<int> _params{cv::IMWRITE_JPEG_QUALITY, 65};
    cv::Size     _sz{320,240};
};

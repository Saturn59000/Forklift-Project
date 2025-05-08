#include "UdpFeedSender.h"
#include <unistd.h>

UdpFeedSender::UdpFeedSender(const char* ip, uint16_t port)
{
    _sock = socket(AF_INET, SOCK_DGRAM, 0);
    _cli.sin_family = AF_INET;
    _cli.sin_port   = htons(port);
    inet_pton(AF_INET, ip, &_cli.sin_addr);
}

UdpFeedSender::~UdpFeedSender() { if(_sock>=0) close(_sock); }

void UdpFeedSender::send(const cv::Mat& frame)
{
    if(frame.empty()) return;
    cv::Mat small; cv::resize(frame, small, _sz);

    std::vector<uchar> jpg;
    cv::imencode(".jpg", small, jpg, _params);
    if(jpg.size() > 64000) return;  // avoid IP fragmentation

    std::vector<uchar> pkt(4 + jpg.size());
    uint32_t net = htonl(++_seq);
    memcpy(pkt.data(), &net, 4);
    memcpy(pkt.data()+4, jpg.data(), jpg.size());

    sendto(_sock, pkt.data(), pkt.size(), 0,
           reinterpret_cast<sockaddr*>(&_cli), sizeof(_cli));
}

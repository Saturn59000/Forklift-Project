#include "stdafx.h"
#include "UdpFeedReceiver.h"
#pragma comment(lib,"ws2_32")

UdpFeedReceiver::UdpFeedReceiver(uint16_t port)
{
    WSADATA w;  WSAStartup(MAKEWORD(2, 2), &w);
    _s = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(port);
    a.sin_addr.s_addr = INADDR_ANY;
    bind(_s, reinterpret_cast<sockaddr*>(&a), sizeof(a));

    u_long nb = 1; ioctlsocket(_s, FIONBIO, &nb);
}

UdpFeedReceiver::~UdpFeedReceiver() { closesocket(_s); WSACleanup(); }

bool UdpFeedReceiver::grab(cv::Mat& out)
{
    int n = recv(_s, _buf, sizeof(_buf), 0);
    if (n <= 4) return false;                 // nothing or too small

    uint32_t seq; memcpy(&seq, _buf, 4); seq = ntohl(seq);
    if (seq <= _lastSeq) return false;        // out‑of‑order / dup

    _lastSeq = seq;
    std::vector<uchar> jpg(_buf + 4, _buf + n);
    out = cv::imdecode(jpg, cv::IMREAD_COLOR);
    return !out.empty();
}


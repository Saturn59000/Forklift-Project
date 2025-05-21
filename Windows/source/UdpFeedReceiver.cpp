#include "stdafx.h"
#include "UdpFeedReceiver.h"

#include <chrono>
#include <thread>
#include <opencv2/opencv.hpp>

#pragma comment(lib, "ws2_32.lib")

 /* ------------------------------------------------------------------ */

constexpr char HELLO[] = "HELLO";        // 5-byte handshake message
constexpr int  BUF_SIZE = 70'000;         // fits one JPEG datagram

UdpFeedReceiver::UdpFeedReceiver(int  localPort,
    const char* piAddr,
    int  piPort)
{
    /*  Initialise Winsock  */
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    /*  Create UDP socket  */
    _sock = socket(AF_INET, SOCK_DGRAM, 0);

    /*  Bind to 0.0.0.0:localPort  */
    sockaddr_in me{};
    me.sin_family = AF_INET;
    me.sin_addr.s_addr = htonl(INADDR_ANY);
    me.sin_port = htons(localPort);
    bind(_sock, reinterpret_cast<sockaddr*>(&me), sizeof(me));

    /*  Store Pi address  */
    _pi.sin_family = AF_INET;
    _pi.sin_addr.s_addr = inet_addr(piAddr);
    _pi.sin_port = htons(piPort);

    /*  Make socket non-blocking  */
    u_long nonBlock = 1;
    ioctlsocket(_sock, FIONBIO, &nonBlock);

    /*  Handshake so Pi learns our address  */
    sendto(_sock,
        HELLO,
        sizeof(HELLO) - 1,      // send exactly 5 bytes
        0,
        reinterpret_cast<sockaddr*>(&_pi),
        sizeof(_pi));
}

/* ------------------------------------------------------------------ */

UdpFeedReceiver::~UdpFeedReceiver()
{
    closesocket(_sock);
    WSACleanup();
}

/* ------------------------------------------------------------------ */
/*  Try to grab next frame; returns true if ‘out’ now holds an image. */
bool UdpFeedReceiver::grab(cv::Mat& out)
{
    char tmp[BUF_SIZE];

    int n = recv(_sock,
        tmp,
        static_cast<int>(sizeof(tmp)),
        0);
    if (n <= 0)
    {
        return false;            // no packet this call
    }

    _buf.assign(tmp, tmp + n);
    out = cv::imdecode(_buf, cv::IMREAD_COLOR);
    return !out.empty();
}

///////////////////////////////////////////////////////////////////
// BCIT ELEX4618 – Reliable TCP client (blocking I/O, 2025-05-15)
///////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

#undef  PI4618
#define WIN4618                      // comment this line when building on Pi

#ifdef WIN4618
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#else                                // ---------- Raspberry Pi -------------
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
typedef int SOCKET;
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#endif

class CClient
{
public:
    CClient();
    ~CClient();

    /* high-level API (unchanged) */
    void  connect_socket(const std::string& addr, int port, double timeout = 1.0);
    void  close_socket();
    void  tx_str(const std::string& s, double timeout = 0.2);   // blocking / retry
    bool  rx_str(std::string& s, double timeout = 1.0);  // reads one line
    bool  rx_im(cv::Mat& im, double timeout = 1.0);  // JPEG image
    SOCKET sock() const { return _sock; }

private:
    SOCKET   _sock{ INVALID_SOCKET };

#ifdef WIN4618
    WSADATA  _wsdat{};
#endif

    /* helpers */
    bool  setBlocking(SOCKET fd, bool block);
    bool  sendAll(const char* buf, size_t len, double timeout);
    bool  recvAll(char* buf, size_t len, double timeout);
};

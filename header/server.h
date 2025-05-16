///////////////////////////////////////////////////////////////////
// BCIT ELEX4618 – Reliable TCP server (blocking I/O, 2025-05-15)
///////////////////////////////////////////////////////////////////
#pragma once

#undef  WIN4618
#define PI4618                               // comment when compiling on PC

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>

#include <opencv2/opencv.hpp>

#ifdef WIN4618
  #include <winsock2.h>
  #pragma comment(lib,"ws2_32.lib")
#else
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR   -1
  typedef int SOCKET;
  #include <sys/types.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <errno.h>
  #include <netinet/tcp.h>
#endif

class CServer
{
public:
    CServer();
    ~CServer();

    void start(int port);                 // call in its own thread
    void stop();

    void get_cmd  (std::vector<std::string>& cmds);
    void send_string(const std::string& s);   // queues outbound line
    void set_txim (cv::Mat& im);              // sets image to transmit

private:
    /* networking */
    SOCKET  _listenSock{ INVALID_SOCKET };
    SOCKET  _cliSock   { INVALID_SOCKET };
    std::atomic<bool> _exit{ false };

    /* queues & buffers */
    std::mutex _rxMtx, _txMtx, _imgMtx;
    std::vector<std::string> _rxList;
    std::vector<std::string> _txList;
    cv::Mat  _txImg;

    /* helpers */
    bool setBlocking(int fd, bool block);
    bool sendAll    (const char* buf, size_t len);
};

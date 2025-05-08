///////////////////////////////////////////////////////////////////
// Prepared for BCIT ELEX4618, April 2022, by Craig Hennessey
///////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <iostream>
#include <cstdint>
#include "Client.h"

#ifdef WIN4618
#pragma comment(lib, "ws2_32.lib")
#endif

#define BUFF_SIZE 65535

CClient::CClient()
{
  _socket = 0;

#ifdef WIN4618
  if (WSAStartup(0x0101, &_wsdat))
  {
    WSACleanup();
    return;
  }
#endif
}

bool CClient::setblocking(SOCKET fd, bool blocking)
{
  if (fd < 0) return false;

#ifdef WIN4618
  unsigned long mode = blocking ? 0 : 1;
  return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0) return false;
  flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
  return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
#endif
}

void CClient::connect_socket(std::string addr, int port)
{
  sockaddr_in ipaddr;

  _socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_socket == SOCKET_ERROR)
  {
#ifdef WIN4618
    WSACleanup();
#endif
    return;
  }

  if (setblocking(_socket, false) == false)
  {
#ifdef WIN4618
    WSACleanup();
#endif
    return;
  }

  ipaddr.sin_family = AF_INET;
  ipaddr.sin_port = htons(port);
  ipaddr.sin_addr.s_addr = inet_addr(addr.c_str());

  connect(_socket, (struct sockaddr*)&ipaddr, sizeof(ipaddr));
}

void CClient::close_socket()
{
#ifdef WIN4618
  closesocket(_socket);
#endif
#ifdef PI4618
  close(_socket);
#endif

  _socket = 0;
}

CClient::~CClient()
{
  close_socket();

#ifdef WIN4618
  WSACleanup();
#endif
}

void CClient::tx_str (std::string str)
{
  send(_socket, str.c_str(), str.length(), 0);
}

bool CClient::recv_all(SOCKET sock, char* data, size_t len, double timeout_sec)
{
    size_t got = 0;
    double start = cv::getTickCount();

    while (got < len)
    {
        int n = recv(sock, data + got, static_cast<int>(len - got), 0);
        if (n > 0) { got += n; continue; }

        if (n == SOCKET_ERROR &&
            WSAGetLastError() == WSAEWOULDBLOCK)
        {
            double dt = (cv::getTickCount() - start) /
                cv::getTickFrequency();
            if (dt < timeout_sec) { std::this_thread::sleep_for(std::chrono::milliseconds(2)); continue; }
        }
        return false;                // timeout or peer closed
    }
    return true;
}

bool CClient::rx_str(std::string& str)
{
    /* ---------- 1. read 4‑byte length header ---------- */
    uint32_t netlen = 0;
    if (!CClient::recv_all(_socket,
        reinterpret_cast<char*>(&netlen), 4))
        return false;

    uint32_t len = ntohl(netlen);
    if (len == 0 || len > 1 << 20)     // 1 MB sanity cap
        return false;

    /* ---------- 2. read the payload ---------- */
    str.resize(len);
    if (!CClient::recv_all(_socket,
        str.data(), len))
        return false;

    return true;                       // ok
}

bool CClient::rx_im(cv::Mat& im)
{
    uint32_t netlen;
    if (!CClient::recv_all(_socket, reinterpret_cast<char*>(&netlen), 4))
        return false;

    uint32_t img_len = ntohl(netlen);
    if (img_len == 0 || img_len > 1 << 22)   // 4 MB sanity
        return false;

    static std::vector<uchar> buf;
    buf.resize(img_len);

    if (!CClient::recv_all(_socket, reinterpret_cast<char*>(buf.data()), img_len))
        return false;

    im = cv::imdecode(buf, cv::IMREAD_UNCHANGED);
    return !im.empty();
}

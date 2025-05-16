///////////////////////////////////////////////////////////////////
// BCIT ELEX4618 – Reliable TCP client implementation
///////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Client.h"
#include <chrono>
#include <thread>

using clk = std::chrono::steady_clock;

/* ───────────────── constructor / destructor ───────────────── */
CClient::CClient()
{
#ifdef WIN4618
    WSAStartup(MAKEWORD(2, 2), &_wsdat);
#endif
}

CClient::~CClient()
{
    close_socket();
#ifdef WIN4618
    WSACleanup();
#endif
}

/* ───────────────── low-level helpers ──────────────────────── */
bool CClient::setBlocking(SOCKET fd, bool block)
{
#ifdef WIN4618
    u_long mode = block ? 0 : 1;
    return ioctlsocket(fd, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return false;
    flags = block ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return fcntl(fd, F_SETFL, flags) == 0;
#endif
}

bool CClient::sendAll(const char* buf, size_t len, double timeout)
{
    auto t0 = clk::now();
    while (len)
    {
        int n = send(_sock, buf, static_cast<int>(len), 0);
        if (n > 0) { buf += n; len -= n; continue; }

#ifdef WIN4618
        if (WSAGetLastError() == WSAEWOULDBLOCK)
#else
        if (errno == EWOULDBLOCK)
#endif
        {
            if (clk::now() - t0 < std::chrono::duration<double>(timeout))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2)); continue;
            }
        }
        return false;                        // hard error or timed-out
    }
    return true;
}

bool CClient::recvAll(char* buf, size_t len, double timeout)
{
    auto t0 = clk::now();
    size_t got = 0;

    while (got < len)
    {
        int n = recv(_sock, buf + got, static_cast<int>(len - got), 0);
        if (n > 0) { got += n; continue; }

#ifdef WIN4618
        if (n == 0 || WSAGetLastError() != WSAEWOULDBLOCK)
#else
        if (n == 0 || errno != EWOULDBLOCK)
#endif
            return false;                    // peer closed or fatal

        if (clk::now() - t0 >= std::chrono::duration<double>(timeout))
            return false;                    // timed-out

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return true;
}

/* ───────────────── public API ─────────────────────────────── */
void CClient::connect_socket(const std::string& addr, int port, double timeout)
{
    close_socket();                                   // in case it was open
    _sock = socket(AF_INET, SOCK_STREAM, 0);
    if (_sock == INVALID_SOCKET) return;

    /* set NON-blocking just for the connect phase */
    setBlocking(_sock, false);

    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = inet_addr(addr.c_str());
    connect(_sock, reinterpret_cast<sockaddr*>(&sa), sizeof(sa));

    /* wait until connect succeeds or fails */
    fd_set wfds;  FD_ZERO(&wfds);  FD_SET(_sock, &wfds);
    timeval tv{};
    tv.tv_sec = static_cast<long>(timeout);
    tv.tv_usec = static_cast<long>((timeout - tv.tv_sec) * 1e6);

    if (select(int(_sock) + 1, nullptr, &wfds, nullptr, &tv) != 1)
    {
        close_socket(); return;
    }

    /* back to BLOCKING mode for reliable send/recv */
    setBlocking(_sock, true);

    /* disable Nagle for low-latency one-liners */
    int flag = 1;
    setsockopt(_sock, IPPROTO_TCP, TCP_NODELAY,
        reinterpret_cast<char*>(&flag), sizeof(flag));
}

void CClient::close_socket()
{
    if (_sock == INVALID_SOCKET) return;
#ifdef WIN4618
    closesocket(_sock);
#else
    close(_sock);
#endif
    _sock = INVALID_SOCKET;
}

/* send full line (blocking), throws away on failure */
void CClient::tx_str(const std::string& s, double timeout)
{
    sendAll(s.c_str(), s.size(), timeout);
}

/* receive up to newline (blocking w/ timeout) */
bool CClient::rx_str(std::string& s, double timeout)
{
    char ch;
    s.clear();
    auto t0 = clk::now();

    while (clk::now() - t0 < std::chrono::duration<double>(timeout))
    {
        int n = recv(_sock, &ch, 1, 0);
        if (n == 1)
        {
            if (ch == '\n') return true;
            s.push_back(ch);
        }
#ifdef WIN4618
        else if (n == 0 || WSAGetLastError() != WSAEWOULDBLOCK)
#else
        else if (n == 0 || errno != EWOULDBLOCK)
#endif
            return false;

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return false;                              // timeout
}

/* blocking JPEG receive – 4-byte length header + data */
bool CClient::rx_im(cv::Mat& im, double timeout)
{
    uint32_t nNet;
    if (!recvAll(reinterpret_cast<char*>(&nNet), 4, timeout)) return false;
    uint32_t n = ntohl(nNet);
    if (n > 2000000 || n < 5000) return false;            // sanity

    std::vector<uchar> buf(n);
    if (!recvAll(reinterpret_cast<char*>(buf.data()), n, timeout)) return false;

    im = cv::imdecode(buf, cv::IMREAD_COLOR);
    return !im.empty();
}

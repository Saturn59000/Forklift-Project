///////////////////////////////////////////////////////////////////
// BCIT ELEX4618 – Reliable TCP server implementation
///////////////////////////////////////////////////////////////////
#include "server.h"
#include <chrono>

using clk = std::chrono::steady_clock;

/* ───────────── constructor / destructor ───────────── */
CServer::CServer()  { _txImg = cv::Mat::zeros(10,10,CV_8UC3); }
CServer::~CServer() { stop(); }

/* ───────────── helper: blocking / non-blocking ─────── */
bool CServer::setBlocking(int fd, bool block)
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

/* ───────────── helper: sendAll (blocking, infinite wait) ───── */
bool CServer::sendAll(const char* buf, size_t len)
{
    while (len)
    {
        int n = send(_cliSock, buf, static_cast<int>(len), 0);
        if (n > 0) { buf += n; len -= n; continue; }

#ifdef WIN4618
        if (WSAGetLastError() == WSAEWOULDBLOCK)
#else
        if (errno == EWOULDBLOCK)
#endif
        { std::this_thread::sleep_for(std::chrono::milliseconds(2)); continue; }

        return false;                      // client closed / fatal
    }
    return true;
}

/* ───────────── public: stop ───────────── */
void CServer::stop()
{
    _exit = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
#ifdef WIN4618
    if (_cliSock   != INVALID_SOCKET) closesocket(_cliSock);
    if (_listenSock!= INVALID_SOCKET) closesocket(_listenSock);
#else
    if (_cliSock   != INVALID_SOCKET) close(_cliSock);
    if (_listenSock!= INVALID_SOCKET) close(_listenSock);
#endif
}

/* ───────────── public: set_txim ───────────── */
void CServer::set_txim(cv::Mat& im)
{
    if (im.empty()) return;
    std::lock_guard<std::mutex> lk(_imgMtx);
    im.copyTo(_txImg);
}

/* ───────────── public: get_cmd ───────────── */
void CServer::get_cmd(std::vector<std::string>& out)
{
    std::lock_guard<std::mutex> lk(_rxMtx);
    out.swap(_rxList);
}

/* ───────────── public: send_string ───────────── */
void CServer::send_string(const std::string& s)
{
    std::lock_guard<std::mutex> lk(_txMtx);
    if (_txList.size() > 200) _txList.clear();      // backlog guard
    _txList.push_back(s);
}

/* ───────────── public: start (blocking loop) ───────────── */
void CServer::start(int port)
{
#ifdef WIN4618
    WSADATA ws; WSAStartup(MAKEWORD(2,2), &ws);
#endif
    _listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenSock == INVALID_SOCKET) return;

    sockaddr_in sa{};
    sa.sin_family      = AF_INET;
    sa.sin_port        = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(_listenSock, reinterpret_cast<sockaddr*>(&sa), sizeof(sa));
    listen(_listenSock, 4);

    /* accept loop */
    while (!_exit)
    {
        fd_set rfds; FD_ZERO(&rfds); FD_SET(_listenSock,&rfds);
        timeval tv{0,100000};                // 100 ms
        select(int(_listenSock)+1,&rfds,nullptr,nullptr,&tv);

        if (FD_ISSET(_listenSock,&rfds))
        {
            _cliSock = accept(_listenSock,nullptr,nullptr);
            if (_cliSock==INVALID_SOCKET) continue;

            /* client socket: blocking, Nagle off */
            setBlocking(_cliSock,true);
            int flag=1; setsockopt(_cliSock,IPPROTO_TCP,TCP_NODELAY,
                     reinterpret_cast<char*>(&flag),sizeof(flag));

            char rxbuff[256];
            std::vector<uchar> jpg;
            std::vector<int> prm{cv::IMWRITE_JPEG_QUALITY,65};

            /* --------------- per-client loop --------------- */
            while (!_exit)
            {
                /* flush outbound */
                {
                    std::lock_guard<std::mutex> lk(_txMtx);
                    for(const std::string& s:_txList)
                        if(!sendAll(s.c_str(),s.size())) { _exit=true; break; }
                    _txList.clear();
                }
                if (_exit) break;

                /* small recv with 50-ms poll */
                fd_set rset; FD_ZERO(&rset); FD_SET(_cliSock,&rset);
                timeval tvr{0,50000};
                int sel = select(int(_cliSock)+1,&rset,nullptr,nullptr,&tvr);

                if (sel==1 && FD_ISSET(_cliSock,&rset))
                {
                    int n = recv(_cliSock,rxbuff,sizeof(rxbuff)-1,0);
                    if (n<=0) break;                     // closed or fatal
                    rxbuff[n]=0;
                    std::string str=rxbuff;

                    if (str=="im")                       // image request
                    {
                        cv::Mat frame;
                        { std::lock_guard<std::mutex> lk(_imgMtx); _txImg.copyTo(frame); }

                        jpg.clear();
                        if (!frame.empty())
                            cv::imencode(".jpg",frame,jpg,prm);

                        uint32_t len = htonl(static_cast<uint32_t>(jpg.size()));
                        if(!sendAll(reinterpret_cast<char*>(&len),4)) break;
                        if(jpg.size() && !sendAll(reinterpret_cast<char*>(jpg.data()),jpg.size())) break;
                    }
                    else                                 // normal command
                    {
                        std::lock_guard<std::mutex> lk(_rxMtx);
                        _rxList.push_back(str);
                    }
                }
            }
            /* client loop ends -> close socket */
#ifdef WIN4618
            closesocket(_cliSock);
#else
            close(_cliSock);
#endif
            _cliSock = INVALID_SOCKET;
        }
    }
#ifdef WIN4618
    WSACleanup();
#endif
}

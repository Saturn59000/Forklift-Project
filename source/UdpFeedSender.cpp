#include "UdpFeedSender.h"

static const int JPEG_QUAL = 65;

void UdpFeedSender::start(int port)
{
    _th = std::thread(&UdpFeedSender::loop, this, port);
}

void UdpFeedSender::stop()
{
    _exit = true;
    if (_th.joinable()) _th.join();
    if (_sock != -1) close(_sock);
}

void UdpFeedSender::setFrame(const cv::Mat& im)
{
    std::lock_guard<std::mutex> lk(_mx);
    if (!im.empty()) im.copyTo(_frame);
}

void UdpFeedSender::loop(int port)
{
    _sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (_sock < 0)
    {
        perror("socket creation failed");
        return;
    }

    sockaddr_in srv{};
    srv.sin_family      = AF_INET;
    srv.sin_addr.s_addr = htonl(INADDR_ANY);
    srv.sin_port        = htons(port);
    if (bind(_sock, (sockaddr*)&srv, sizeof(srv)) < 0)
    {
        perror("bind failed");
        close(_sock);
        return;
    }

    // --- Wait for first “HELLO” from Windows so we know where to stream ---
    char buf[8];
    socklen_t len = sizeof(_clientAddr);
    while (!_exit && !_haveClient)
    {
        int n = recvfrom(_sock, buf, sizeof(buf), MSG_DONTWAIT,
                         (sockaddr*)&_clientAddr, &len);

        if (n == 4 || n == 5) // accept “HELO” or “HELLO”
        {
            _haveClient = true;
            printf("Handshake from %s:%d (%.*s)\n",
                   inet_ntoa(_clientAddr.sin_addr),
                   ntohs(_clientAddr.sin_port), n, buf);
        }
        usleep(50'000);
    }

    std::vector<uchar> jpg;
    std::vector<int> prm{cv::IMWRITE_JPEG_QUALITY, JPEG_QUAL};

    while (!_exit)
    {
        if (!_haveClient)
        {
            usleep(100'000);
            continue;
        }

        cv::Mat f;
        {
            std::lock_guard<std::mutex> lk(_mx);
            if (_frame.empty())
            {
                usleep(1'000);
                continue;
            }
            f = _frame.clone();
        }

        cv::imencode(".jpg", f, jpg, prm);
        ssize_t sent = sendto(_sock, jpg.data(), jpg.size(), 0,
                              (sockaddr*)&_clientAddr, sizeof(_clientAddr));

        if (sent < 0)
        {
            perror("sendto failed");
            _haveClient = false;  // Client might be dead
            usleep(500'000);      // Wait before retrying
        }

        usleep(66'000); // ~15 fps
    }
}

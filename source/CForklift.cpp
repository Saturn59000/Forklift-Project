#define CVUI_IMPLEMENTATION
#include "CAruco.h"



#define WINDOW_NAME "Forklift – Server"
#define JPEG_QUAL = 65;

#define WINDOW_NAME "Forklift – Pi side"
#define PORT_FEED 4618
#define PORT_CMD 4620
#define JPEG_QUAL 65

enum aruco_ids
{
    TRUCK1 = 1,
    TRUCK2 = 2, 
    TRUCK3 = 3,
    TRUCK4 = 4,
    PACKAGE1 = 5,
    PACKAGE2 = 6
};


CForklift::CForklift()
{
    _canvas = cv::Mat(cv::Size(500, 380), CV_8UC3);
    cvui::init(WINDOW_NAME);

    if (gpioInitialise() < 0)
    {
        throw std::runtime_error("pigpio init failed");
    }
    
    _cap.open(0);
    
    gpioSetMode(_servoGpio, PI_OUTPUT);
    gpioServo(_servoGpio, _pulseDown);   // start in “Down” position

    std::thread([&]{ _udp.start(PORT_FEED); }).detach();
    std::thread([&]{ _srvCmd .start(PORT_CMD ); }).detach();
}

CForklift::~CForklift()
{
    _drive.stop();
    _srvCmd.stop();
    if (_cap.isOpened()) _cap.release();
    cv::destroyAllWindows();
    gpioTerminate();
    gpioServo(_servoGpio, 0);   // turn PWM off
}


/* ───────────── logging helpers ───────────── */
void CForklift::pushLog(std::string s)
{
    std::lock_guard<std::mutex> lk(_logMtx);
    _log.push_front(std::move(s));
    if (_log.size() > 8) _log.pop_back();
}


/* ───────────── handle commands from Windows ───────────── */
void CForklift::handleCommands()
{
    std::vector<std::string> cmds;
    _srvCmd.get_cmd(cmds);
    if (cmds.empty()) return;

    _clientSeen = true;
    _lastCmdT   = std::chrono::steady_clock::now();

    for (auto& raw : cmds)
    {
        std::string s = raw;
        s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
        s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
        pushLog(s);

        if      (s == "MODE AUTO")   _autoMode = true;
        else if (s == "MODE MANUAL") _autoMode = false;
        else if (!_autoMode)
        {                    // manual only
            if      (s == "UP")    _drive.forward();
            else if (s == "DOWN")  _drive.backward();
            else if (s == "LEFT")  _drive.left();
            else if (s == "RIGHT") _drive.right();
            else if (s == "STOP")  _drive.stop();
            else if (s == "FORKUP")   gpioServo(_servoGpio, _pulseUp);
            else if (s == "FORKDOWN") gpioServo(_servoGpio, _pulseDown);
            else if (s.rfind("SPD",0)==0)
            {
                int v = std::stoi(s.substr(3));
                _speed = v;
            }
        }
        _srvCmd.send_string("ACK " + s + "\n");
    }
}


/* ───────────── update ───────────── */
void CForklift::update()
{
    if (_exit) return;

    // camera -> feed 
    if(_cap.isOpened())
    {
    _cap.read(_frame);
    cv::Mat no_rotate;
    _vid >> no_rotate;
    cv::rotate(no_rotate, _frame, cv::ROTATE_180);
    }

    if (!_frame.empty())
    {
        cv::Mat small; cv::resize(_frame, small, _FEED_SIZE);
        _udp.setFrame(small);
    }
        
    /****************** MANUAL MODE*********************/
    handleCommands();

    // stop motors if no client for 3 s in manual 
    if (!_autoMode && _clientSeen && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - _lastCmdT).count() > 3)
    {
        _drive.stop();
        _clientSeen = false;
    }

    static int frameCtr = 0;
    if (++frameCtr % 2 == 0) _drive.tick();   // tick every 2nd frame

    /******************** AUTO MODE *********************/
    if (_autoMode)
    {
        //detect aruco
        _aruco.detect_markers(_frame);
        _aruco.draw_markers(_frame);
        std::vector<int> ids = _aruco.get_marker_ids();
    
        //find first truck box id
        //test CNavigate timing
        if (!_run_once)
        {
        _nav.forward(5);
        _run_once = true; 
        }
   

    }

    send_frame(_frame); //send frame to camera with aruco ids
    
}


/* ───────────── draw ───────────── */
void CForklift::draw()
{
    _canvas = cv::Scalar(40, 40, 40);

    cvui::text(_canvas, 20, 20, "Windows link:");
    cvui::printf(_canvas, 150, 20, 0.6,
                 _clientSeen ? 0x00FF00 : 0xFF4444,
                 _clientSeen ? "CONNECTED" : "waiting...");

    bool autoBox = _autoMode, manBox =! _autoMode;

    if (cvui::checkbox(_canvas, 20, 60, "Auto Mode",&autoBox)) _autoMode=autoBox;
    if (cvui::checkbox(_canvas, 20, 90, "Manual Mode",&manBox)) _autoMode=!manBox;

    int y0=150, bw=70, bh=30;

    if (!_autoMode)
    {
        /* ---- press‑and‑hold detection using mouse coordinates ---- */
        auto inside = [&](int x, int y, int w, int h)
        {
            int mx = cvui::mouse().x;
            int my = cvui::mouse().y;
            return mx >= x && mx <= x + w && my >= y && my <= y + h;
        };

        bool mouseDown = cvui::mouse(cvui::IS_DOWN);   // any mouse button
        int  dir = 0;                                  // 0 = none

        if (mouseDown)
        {
            if      (inside( 90, y0,      bw, bh)) dir = 1; // UP
            else if (inside( 90, y0 + 40, bw, bh)) dir = 2; // DOWN
            else if (inside( 20, y0 + 40, bw, bh)) dir = 3; // LEFT
            else if (inside(160, y0 + 40, bw, bh)) dir = 4; // RIGHT
            else if (inside( 90, y0 + 80, bw, bh)) dir = 5; // STOP
        }

        static int lastDir = 0;

        if (dir != lastDir)
        {
            switch (dir)
            {
            case 1: _drive.forward();   break;
            case 2: _drive.backward();  break;
            case 3: _drive.left();      break;
            case 4: _drive.right();     break;
            case 5: _drive.stop();      break;
            default: _drive.stop();     break;   // mouse released or outside
            }
            lastDir = dir;
        }

        // ---- draw the buttons ----
        cvui::button(_canvas,  90, y0,      bw, bh, "UP");
        cvui::button(_canvas,  20, y0 + 40, bw, bh, "LEFT");
        cvui::button(_canvas,  90, y0 + 40, bw, bh, "DOWN");
        cvui::button(_canvas, 160, y0 + 40, bw, bh, "RIGHT");
        cvui::button(_canvas,  90, y0 + 80, bw, bh, "STOP");

        /* ----- Current speed display ----- */
        cvui::printf(_canvas, 300, 250, 0.6, 0x00ffff, "Speed: %3d", int(_speed));
        _drive.setSpeed(int(_speed));
    }
    else
    {
        cvui::text(_canvas, 30, y0 + 30, "Buttons disabled in Auto");
    }


    /* ─── Fork-lift servo buttons ─── */
    int yServo = y0 + 130;

    if (cvui::button(_canvas, 45, yServo, bw+10, bh, "Fork Up"))
    {
        gpioServo(_servoGpio, _pulseUp);
    }

    if (cvui::button(_canvas, 125, yServo, bw+10, bh, "Fork Down"))
    {
        gpioServo(_servoGpio, _pulseDown);
    }

    /* cmd log */
    cvui::text(_canvas, 300, 20, "CMD Log");
    int y=45;

    {
        std::lock_guard<std::mutex> lk(_logMtx);
        for(auto& s:_log) { cvui::printf(_canvas, 300, y, 0.5, 0xFFFFFF, "%s", s.c_str()); y+=18; }
    }

    if (cvui::button(_canvas, 20, 330, 80, 30, "Exit")) _exit=true;

    cvui::update();
    cv::imshow(WINDOW_NAME, _canvas);
}

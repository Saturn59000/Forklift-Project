#include "CCamera.h"

CCamera::CCamera(){};

CCamera::~CCamera()
{
    close();
}
void CCamera::open(int camID)
{
    std::string pipeline = "libcamerasrc ! video/x-raw,width=640,height=480,framerate=30/1 ! videoconvert ! appsink";
    _vid.open(pipeline, cv::CAP_GSTREAMER);
    
    if (!_vid.isOpened())
    {
        std::cerr << "Error: Unable to open camera " << camID << std::endl;
    }
}
void CCamera::close()
{
    if (_vid.isOpened())
    {
        _vid.release();
    }
}
void CCamera::capture(cv::Mat &frame)
{
    if (_vid.isOpened() == true)
    {
        _vid >> frame;
    }
}
#include "CCamera.h"

CCamera::CCamera(){};

CCamera::~CCamera()
{
    close();
}
void CCamera::open(int camID)
{
    _vid.open(camID);
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
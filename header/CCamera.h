#pragma once

#include <opencv2/opencv.hpp>

class CCamera
{
    private: 
    cv::VideoCapture _vid;

    public:
    CCamera();
    ~CCamera();

    void open(int camID);
    void close();
    void capture(cv::Mat &frame);
};
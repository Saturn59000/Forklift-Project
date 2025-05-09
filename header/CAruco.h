#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>

class CAruco 
{
    private:
    cv::Ptr<cv::aruco::Dictionary>  _dictionary;
    std::vector<int> _marker_ids;
    std::vector<std::vector<cv::Point2f>> _marker_corners;

    public:
    CAruco();
    ~CAruco();

    void detect_markers(cv::Mat frame);
    void draw_markers(cv::Mat frame);

    std::vector<int> get_marker_ids() { return _marker_ids; };
    std::vector<std::vector<cv::Point2f>> get_marker_corners() { return _marker_corners; };

};

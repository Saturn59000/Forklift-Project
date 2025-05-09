#include "CAruco.h"


CAruco::CAruco()
{
_dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
}

void CAruco::detect_markers(cv::Mat frame)
{
    cv::aruco::detectMarkers(frame, _dictionary, _marker_corners, _marker_ids);
}

void CAruco::draw_markers(cv::Mat frame)
{
    if (!_marker_ids.empty())
        cv::aruco::drawDetectedMarkers(frame, _marker_corners, _marker_ids);
    
    cv::imshow("Detectedmarkers", frame);
    cv::waitKey(10);
}

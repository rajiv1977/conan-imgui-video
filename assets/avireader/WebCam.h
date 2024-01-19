#pragma once

#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencv2/imgcodecs.hpp"
#include <opencv2/highgui.hpp>
#include "opencv2/videoio.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/core/types_c.h>
#include "opencv2/highgui/highgui.hpp"

#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>

namespace ImGui
{
class WebCam
{
  public:
    WebCam();
    ~WebCam();

    void    initialize();
    bool    readFrame();
    cv::Mat getImageFrame() const;

  private:
    void nnmTracking(cv::Mat& frame);

    cv::VideoCapture         mCaption;
    std::vector<std::string> mClassNames;
    cv::Mat                  mFrame;
    cv::dnn::Net             mNeuralNetworkModel;
    bool                     mWebCamStatus;
};
} // namespace ImGui

#pragma once

#include "IconsFontAwesome5.h"
#include "VideoSensorDriverVisualization.h"

class ImGuiVis : public VisualizationBase
{
  public:
    ImGuiVis(const std::string& camSource);
    ~ImGuiVis();

  private:
    // Process the data
    void run();

    // ImGui initialization
    GLFWwindow* initFcn();

    // Callback functions.
    static void closeRequestCallback(GLFWwindow* window);
    static void glfw_error_callback(int error, const char* description);
    std::string displayTime();

    // Clean up
    void cleanUp();
    void Distroy(GLFWwindow* window);

    // Additional
    void getFontIcon();
    void toLower(std::string& data);

  private:
    VIDEO::VideoSensorDriverVisualization& mVideoVisualization;

    // Declaring variables
    std::string   mOpenGLVersion = "Unavailable";
    GLFWwindow*   mWorldWindow;
    GLuint        mWorldTexture;
    bool          mPauseRequested;
    float         mBaseFontSize; // 13.0f is the size of the default font. Change to the font size you use.
    std::mutex    mFontIcon;
    bool          mWebCamStatus;
    std::string   mAVIFileName;
    float         mAVIFreq;
    ImGui::WebCam mWebCam;
    H264::Decoder mDecoder;
};

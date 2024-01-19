#pragma once

#include "UTILITYMath.h"
#include "Decoder.h"
#include "VisualizationBase.h"
#include "WebCam.h"

namespace VIDEO
{

enum
{
    LVIO_SUCCESS        = 0,
    LVIO_EOF            = 1,
    LVIO_FILEIOERR      = 2,
    LVIO_DECODE_FAILURE = 3
};

enum VideoIOFrameType_e
{
    LVIO_YUV422 = 0,
    LVIO_RGB_8BPP
};

enum MotionType_e
{
    STATIC, // Stationary entity
    MOVING, // Nonstationary entity
    ALL     // Can be static or moving entity
};

// display options
struct WorldFrameSettings_t
{
    bool displayCamera;
    WorldFrameSettings_t()
        : displayCamera(false)
    {
    }
};

// Image frame data
struct VisualizerImageFrame_t
{
    mutable std::mutex lock;
    uint32_t           width;
    uint32_t           height;
    uint32_t           stride;
    uint32_t           frameNumber;
    uint8_t*           pixels;
    uint32_t           decodeStatus;
    VideoIOFrameType_e type;
    int64_t            fps;
};

class VideoSensorDriverVisualization : public VisualizationBase
{
  private:
    // Definition for shader data for each image
    struct VideoData_t
    {
        std::mutex                    dataMutex;
        bool                          isInitialized = false;
        GLuint                        imageTexture  = 0;
        const VisualizerImageFrame_t* frame         = nullptr;
    };

  public:
    static VideoSensorDriverVisualization& getInstance();

    // Get the vehicle's calibration info
    void initialize();

    // Set the worldvis functions
    WorldFrameSettings_t mWorldFrameSettings;
    void                 worldFramePreDisplayFcn(bool demoMode) override;

    // Set data functions
    void setWebCameraStatus(const bool& status);
    void setVideoFrame(const VisualizerImageFrame_t& ImageFrame);

    bool getWebCameraStatus() const;

    // world to camera coordinate transformation
    UTILITY::Vector3f_t worldToCameraCoordinate(const UTILITY::Vector3f_t& worldPos);

    VisualizerImageFrame_t mLibVideoIOFrame;
    bool                   mWebCamOn;

    void        getWebCamFrame(const cv::Mat& frame);
    void        getRGBFrame(int      width,
                            int      height,
                            uint8_t* data1,
                            uint8_t* data2,
                            uint8_t* data3,
                            int*     lineSize,
                            int      displayPictureNumber,
                            float    samplingTime);
    inline int  pixelSaturate(int v, int minimum, int maximum);
    uint8_t*    processRGB(const uint8_t* Yo,
                           const uint8_t* Uo,
                           const uint8_t* Vo,
                           uint32_t       cols,
                           uint32_t       rows,
                           uint32_t       strideOut,
                           uint32_t       strideY,
                           uint32_t       strideU,
                           uint32_t       strideV);
    std::string getTime();
    std::string getDayAndTime();

    // Constructor
    VideoSensorDriverVisualization();

    // Destructor
    ~VideoSensorDriverVisualization();

    // video frame data
    VideoData_t mVideo;

    // Data mutex to be used whenever reading/writing to data members
    std::mutex mVisionLock;

    bool mWebCamStatus;

    // Initial configuration settings
    void                 worldReadIniSettings() override;
    WorldFrameSettings_t mInitialWorldFrameSettings;

    // Common functions image processing
    void                                initializeFrame(VideoData_t& videoData);
    void                                localFrameDisplayFcn() override;
    void                                updateFrame(VideoData_t& videoData, const VisualizerImageFrame_t& frame);
    std::array<UTILITY::Vector2f_t, 2U> computeCurrentImageSize(const VideoData_t& frame);
    void                                drawFrame(VideoData_t& videoData);
    void                                cleanupFrame(VideoData_t& videoData);

    // world to pixel coordinate transformation
    UTILITY::Vector2f_t              worldToPixelCoordinates(const UTILITY::Vector3f_t& position_m,
                                                             const UTILITY::Vector2f_t& principalPoint_pixel,
                                                             const UTILITY::Vector3f_t& cameraPosition_m,
                                                             float32_t                  focalLength_m = 0.00629F, // FOV = 100deg
                                                             float32_t                  pixelSize_m = 0.0000042F,
                                                             float32_t                  roll_rad    = 0.0F,
                                                             float32_t                  pitch_rad   = 0.0F,
                                                             float32_t                  yaw_rad     = 0.0F,
                                                             float32_t                  alpha       = 0.0F);
    std::vector<UTILITY::Vector2f_t> polyLineClipping(const std::vector<UTILITY::Vector2f_t>& lines1,
                                                      const std::vector<UTILITY::Vector2f_t>& lines2);
    ImVec2                           formatToImVec2(const UTILITY::Vector2f_t& vec);
    UTILITY::Vector2f_t              ImVec2ToVector2f(const ImVec2& vec);
};

} // namespace VIDEO
